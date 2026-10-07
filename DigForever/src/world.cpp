#include "world.h"

#include <algorithm>
#include <cassert>
#include <cmath>

// ========================================================
// grid generation
// ========================================================

Tile World::makeRandomTile() noexcept {
    const int randDistrNum = m_tileDistr(m_rng);
    const TileType type = static_cast<TileType>(randDistrNum);
    const int numHits = 
        (randDistrNum < static_cast<int>(TileType::TILE_TOUGH)) 
        ? kNUM_HITS_NORMAL : 
        (randDistrNum < static_cast<int>(TileType::TILE_HEALTH))
        ? kNUM_HITS_TOUGH : kNUM_HITS_HEALTH;
        
    return Tile{ type, numHits };   
}

void World::generateInitialGrid() noexcept {
    for (int col = 0; col < kCOLS; ++col) {
        for (int row = 0; row < kROWS; ++row) {
            if (row <= kCENTER_ROW) {
                setTile(col, row, Tile{}); // default empty tile
            } 
            else {
                const Tile tile = makeRandomTile();
                setTile(col, row, tile);
            }
        }
    }    
}

void World::clearGrid() noexcept {
    for (int row = 0; row < kROWS; ++row) {
        for (int col = 0; col < kCOLS; ++col) {
            m_grid[row][col] = Tile{};
        }
    }

    m_fallingTiles.clear();
    m_bottomRowDepth = m_depth;
}

// ========================================================
// groups
// ========================================================

World::TileGroup World::findGroup(int col, int row) const noexcept {
    TileGroup group;

    if (!inGridBounds(col, row)) { return group; }

    const TileType type = m_grid[row][col].type;
    if (type == TileType::TILE_EMPTY) { return group; }

    if (type == TileType::TILE_TOUGH) {
        group.tiles[0] = GridPos{ col, row };
        group.count = 1;
        return group;
    }

    std::array<std::array<bool, kCOLS>, kROWS> visited {};  // keep track of visited tiles to avoid double counting
    std::array<GridPos, kCOLS * kROWS> toVisit;             // each GridPos here used in tryAddTile
    int numToVisit = 0;

    auto tryAddTile = [&](int tileX, int tileY) -> void {
        if (!inGridBounds(tileX, tileY))         { return; }
        if (visited[tileY][tileX])               { return; }
        if (m_grid[tileY][tileX].type != type)   { return; }

        visited[tileY][tileX] = true;
        toVisit[numToVisit] = { tileX, tileY };
        ++numToVisit;
    };


    tryAddTile(col, row);

    while (numToVisit > 0) {
        --numToVisit;
        const GridPos current = toVisit[numToVisit];

        group.tiles[group.count] = current;
        ++group.count;

        tryAddTile(current.x + 1, current.y    );
        tryAddTile(current.x - 1, current.y    );
        tryAddTile(current.x,     current.y + 1);
        tryAddTile(current.x,     current.y - 1);
    }

    return group;
}

int World::countMatchingGroup(int col, int row) const noexcept {
    return findGroup(col, row).count;
}

void World::destroyGroup(int col, int row) noexcept {
    const TileGroup group = findGroup(col, row);
    for (int i = 0; i < group.count; ++i) {
        const GridPos& tile = group.tiles[i];
        m_grid[tile.y][tile.x] = Tile{}; 
    }

    updateScore(group.count);
}

// ========================================================
// floating
// ========================================================

bool World::isReserved(int col, int row) const noexcept {
    for (const FallingTile& fallingTile : m_fallingTiles) {
        if (fallingTile.col == col && fallingTile.targetRow == row) {
            return true;
        }
    }

    return false;
}

bool World::isSupportedFromBelow(int col, int row, TileType groupType) const noexcept {
    if (row >= kROWS - 1) { return true; }

    const TileType below = m_grid[row + 1][col].type;
    if (below == groupType)           { return false; }
    if (below != TileType::TILE_EMPTY) { return true; }

    return isReserved(col, row + 1);
}

bool World::groupIsFloating(int col, int row) const noexcept {
    if (!inGridBounds(col, row)) { return false; }

    const TileType type = m_grid[row][col].type;
    if (type == TileType::TILE_EMPTY || type == TileType::TILE_TOUGH) { return false; }

    const TileGroup group = findGroup(col, row);
    for (int i = 0; i < group.count; ++i) {
        const GridPos& tile = group.tiles[i];
        if (isSupportedFromBelow(tile.x, tile.y, type)) {
            return false;   // one supported tile holds up the whole group
        }
    }

    return true;
}

// ========================================================
// falling
// ========================================================

int World::findLandingRow(int col, int row) const noexcept {
    for (int r = row + 1; r < kROWS; ++r) {
        if (m_grid[r][col].type != TileType::TILE_EMPTY || isReserved(col, r)) {
            return r - 1;
        }
    }

    return kROWS - 1;
}  

int World::landTile(const FallingTile& fallingTile) noexcept {
    // Normally the target is empty because it was reserved
    // settle in the nearest empty cell above it instead of overwriting
    int row = fallingTile.targetRow;
    while (row >= 0 && m_grid[row][fallingTile.col].type != TileType::TILE_EMPTY) {
        --row;
    }
    if (row >= 0) {
        m_grid[row][fallingTile.col] = fallingTile.tile;
    }

    return row;
}

bool World::targetIsUnsupported(const FallingTile& fallingTile) noexcept {
    const int col = fallingTile.col;
    const int row = fallingTile.targetRow;

    if (row < 0 || row >= kROWS - 1)                    { return false; }
    if (m_grid[row][col].type != TileType::TILE_EMPTY)  { return false; }

    m_grid[row][col] = fallingTile.tile;        // pretend it landed
    const bool floating = groupIsFloating(col, row);
    m_grid[row][col] = Tile{};                  // undo

    return floating;
}

bool World::updateFallingTile(FallingTile& fallingTile, float dt, int& landedRow) noexcept {
    landedRow = -1;

    // wobble in place before dropping
    if (fallingTile.fallTimer < FallingTile::kTIME_BEFORE_FALL) {
        fallingTile.fallTimer += dt;
        return false;
    }

    fallingTile.fallOffset += FallingTile::kFALL_SPEED * dt;

    const float landingOffset = 
        static_cast<float>((fallingTile.targetRow - fallingTile.startRow) * kTILE_SIZE);
    if (fallingTile.fallOffset < landingOffset) {
        return false;
    }

    // support below the target was removed while falling: aim lower and keep going
    if (targetIsUnsupported(fallingTile)) {
        const int newTargetRow = findLandingRow(fallingTile.col, fallingTile.targetRow);
        if (newTargetRow > fallingTile.targetRow) {
            fallingTile.targetRow = newTargetRow;
            return false;
        }
    }

    landedRow = landTile(fallingTile);
    const int groupSize = countMatchingGroup(fallingTile.col, landedRow);
    if (landedRow >= 0 && groupSize >= kMIN_GROUP_TO_CLEAR) {
        destroyGroup(fallingTile.col, landedRow); // adds 1^multiplier to score
        updateScore(groupSize * 2);               // additional 2^multiplier to score
    }

    return true;
}

void World::startFallingTiles() {
    for (int col = 0; col < kCOLS; ++col) {
        for (int row = kROWS - 2; row >= 0; --row) {
            if (!groupIsFloating(col, row)) {
                continue;
            }

            const int targetRow = findLandingRow(col, row);
            if (targetRow <= row) {
                continue;
            }
            
            // this tile becomes a falling tile
            m_fallingTiles.emplace_back(
                FallingTile{
                    m_grid[row][col], 
                    col, row, targetRow 
                }
            );

            // reset where this tile used to be located
            m_grid[row][col] = Tile{};
        }
    }
}




// ========================================================
// Score calculation
// ========================================================

void World::updateScore(int numDestroyedTile) noexcept {
    m_scoreThisFrame += static_cast<int>(
        std::powf(static_cast<float>(numDestroyedTile), 1.25f)) * 10;
}

// ========================================================
// Static methods
// ========================================================

float World::gridToWorldPosX(int col) noexcept {
    assert(inGridBounds(col, 0));
    return static_cast<float>(col * kTILE_SIZE);
}

float World::gridToWorldPosY(int row) noexcept {
    assert(inGridBounds(0, row));
    return static_cast<float>(row * kTILE_SIZE) + World::s_yOffset;
}

int World::worldToGridPosX(float x) noexcept {
    return static_cast<int>(x / kTILE_SIZE);    
}

int World::worldToGridPosY(float y) noexcept {
    return static_cast<int>( (y - s_yOffset) / kTILE_SIZE);
}

float World::fallingTileWorldPosY(const FallingTile& fallingTile) noexcept {
    return static_cast<float>(fallingTile.startRow * kTILE_SIZE) + 
            fallingTile.fallOffset + World::s_yOffset;
}

bool World::inGridBounds(int col, int row) noexcept {
    return col >= 0 && col < kCOLS && row >= 0 && row < kROWS;
}

void World::shiftOffsetY(float dy) noexcept {
    World::s_yOffset += dy;
}

float World::getOffsetY() noexcept {
    return World::s_yOffset;
}

// ========================================================
// exposed for game
// ========================================================

std::vector<World::GridPos> World::updateFallingTiles(float dt) {
    startFallingTiles();
    std::vector<GridPos> justLandedCoordinates {};

    for (std::size_t i = 0; i < m_fallingTiles.size();) {
        int landedRow = -1;
        const bool landed = updateFallingTile(m_fallingTiles[i], dt, landedRow);
        if (landed) {
            if (landedRow > -1) {
                justLandedCoordinates.push_back(GridPos{m_fallingTiles[i].col, landedRow});
            }
            m_fallingTiles[i] = m_fallingTiles.back();   // swap-and-pop removal
            m_fallingTiles.pop_back();
        }
        else {
            ++i;
        }
    }

    return justLandedCoordinates;
}

bool World::clearTilesForRespawn(int col, int row) noexcept {
    if (!m_fallingTiles.empty()) { return false; }

    constexpr int kMinCol = 1;
    constexpr int kMaxCol = kCOLS - 2;

    const int clampedX = std::max(kMinCol, std::min(col, kMaxCol));
    for (int c = clampedX - 1; c <= clampedX + 1; ++c) {
        for (int r = 0; r <= row; ++r) {
            m_grid[r][c] = Tile{};
        }
    }

    return true;
}

bool World::hitTile(int col, int row) noexcept {
    assert(inGridBounds(col, row));
    Tile& tile = m_grid[row][col];
    if (tile.type == TileType::TILE_EMPTY || --tile.numHits > 0) {
        return false;
    }

    destroyGroup(col, row);
    return true;
}


void World::buildOneRow() {
    // replace every row with the row beneath it. skipping the last row
    for (int row = 0; row < kROWS - 1; ++row) {
        m_grid[row] = m_grid[row + 1];
    }

    for (FallingTile& fallingTile : m_fallingTiles) {
        --fallingTile.startRow;
        --fallingTile.targetRow;
    }

    m_fallingTiles.erase(
        std::remove_if(m_fallingTiles.begin(), m_fallingTiles.end(),
                        [](const FallingTile& f) { return f.targetRow < 0; }),
        m_fallingTiles.end());

    const int sectionDepth = (m_depth / kROWS_PER_SECTION + 1) * kROWS_PER_SECTION;
    if (m_bottomRowDepth - m_depth >= kROWS_BELOW_PLAYER) {
        ++m_depth;
    }

    if (m_bottomRowDepth >= sectionDepth) {
        const int floorRow = (kROWS - 2) - (m_bottomRowDepth - sectionDepth);
        if (floorRow <= kCENTER_ROW) {
            clearGrid();
        }
    }

    ++m_bottomRowDepth;
    const bool isTough = m_bottomRowDepth >= (m_depth / kROWS_PER_SECTION + 1) * kROWS_PER_SECTION;

    for (int col = 0; col < kCOLS; ++col) {
        const Tile tile = isTough
                        ? Tile{ TileType::TILE_TOUGH, kNUM_HITS_TOUGH }
                        : makeRandomTile();
        setTile(col, kROWS - 1, tile);
    }
}

int World::collectHealthTiles(float x, float y, float w, float h) noexcept {
    // repeated logic from Player::collidesAt(float x, float y, float w, float h)

    constexpr float kEdge = 0.01f;

    const int leftCol   = worldToGridPosX(x);
    const int rightCol  = worldToGridPosX(x + w - kEdge);
    const int topRow    = worldToGridPosY(y);
    const int bottomRow = worldToGridPosY(y + h - kEdge);

    int collected = 0;
    for (int row = topRow; row <= bottomRow; ++row) {
        for (int col = leftCol; col <= rightCol; ++col) {
            if (!inGridBounds(col, row)) { continue; }

            if (m_grid[row][col].type == TileType::TILE_HEALTH) {
                m_grid[row][col] = Tile{};
                ++collected;
            }
        }
    }

    return collected;
}

// ========================================================
// getters / setters
// ========================================================

void World::setTile(int col, int row, Tile type) noexcept {
    assert(inGridBounds(col, row));
    m_grid[row][col] = type;
}

const Tile& World::getTile(int col, int row) const noexcept {
    assert(inGridBounds(col, row));
    return m_grid[row][col];
}

const std::array<std::array<Tile, World::kCOLS>, World::kROWS>& World::getTiles() const noexcept { 
    return m_grid;
}

const std::vector<FallingTile>& World::getFallingTiles() const noexcept {
    return m_fallingTiles;
}

int World::getScoreThisFrame() noexcept {
    const int toReturn = m_scoreThisFrame;
    m_scoreThisFrame = 0;

    return toReturn;
}

int World::getDepth() const noexcept {
    return m_depth;
}

// class

World::World() {
    generateInitialGrid();
}