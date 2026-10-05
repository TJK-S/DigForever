#include "world.h"

#include <algorithm>
#include <random>
#include <cassert>

// ========================================================
// grid generation
// ========================================================

Tile World::makeRandomTile() {
    constexpr int defaultNumHits { 1 };
    constexpr int toughtNumHits  { 5 };

    const int randDistrNum = m_tileDistr(m_rng);
    const TileType type = static_cast<TileType>(randDistrNum);
    const int numHits = 
        (randDistrNum < static_cast<int>(TileType::TILE_TOUGH)) 
        ? defaultNumHits : toughtNumHits;
        
    return Tile{ type, numHits };   
}

void World::generateInitialGrid() {
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

// ========================================================
// groups
// ========================================================

World::TileGroup World::findGroup(int col, int row) const {
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

int World::countMatchingGroup(int col, int row) const {
    return findGroup(col, row).count;
}

void World::destroyGroup(int col, int row) {
    const TileGroup group = findGroup(col, row);
    for (int i = 0; i <group.count; ++i) {
        const GridPos& tile = group.tiles[i];
        m_grid[tile.y][tile.x] = Tile {}; 
    }
}

// ========================================================
// floating
// ========================================================

bool World::isReserved(int col, int row) const {
    for (const FallingTile& fallingTile : m_fallingTiles) {
        if (fallingTile.col == col && fallingTile.targetRow == row) {
            return true;
        }
    }

    return false;
}

bool World::isSupportedFromBelow(int col, int row, TileType groupType) const {
    if (row >= kROWS - 1) { return true; }

    const TileType below = m_grid[row + 1][col].type;
    if (below == groupType)           { return false; }
    if (below != TileType::TILE_EMPTY) { return true; }

    return isReserved(col, row + 1);
}

bool World::groupIsFloating(int col, int row) const {
    if (!inGridBounds(col, row)) { return false; }

    const TileType type = m_grid[row][col].type;
    if (type == TileType::TILE_EMPTY || type == TileType::TILE_TOUGH) { return false; }


    std::array<std::array<bool, kCOLS>, kROWS> visited {};  // keep track of visited tiles to avoid double counting
    std::array<GridPos, kCOLS * kROWS> toVisit; // each pair here used in tryAddTile
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

    // visit tiles anc check if they are supported below.
    // A visit means that this tile is good and we should check
    // adjacent tiles too see if they are also good and then visit them as well
    while (numToVisit > 0) {
        --numToVisit;
        const GridPos current = toVisit[numToVisit];

        if (isSupportedFromBelow(current.x, current.y, type)) {
            return false;
        }

        tryAddTile(current.x + 1, current.y    );
        tryAddTile(current.x - 1, current.y    );
        tryAddTile(current.x,     current.y + 1);
        tryAddTile(current.x,     current.y - 1);
    }

    return true;
}

// ========================================================
// falling
// ========================================================

int World::findLandingRow(int col, int row) const {
    for (int r = row + 1; r < kROWS; ++r) {
        if (m_grid[r][col].type != TileType::TILE_EMPTY || isReserved(col, r)) {
            return r - 1;
        }
    }

    return kROWS - 1;
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

bool World::targetIsUnsupported(const FallingTile& fallingTile) {
    const int col = fallingTile.col;
    const int row = fallingTile.targetRow;

    if (row < 0 || row >= kROWS - 1)                    { return false; }
    if (m_grid[row][col].type != TileType::TILE_EMPTY)  { return false; }

    m_grid[row][col] = fallingTile.tile;        // pretend it landed
    const bool floating = groupIsFloating(col, row);
    m_grid[row][col] = Tile{};                  // undo

    return floating;
}

int World::landTile(const FallingTile& fallingTile) {
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

bool World::updateFallingTile(FallingTile& fallingTile, float dt, int& landedRow) {
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
    if (landedRow >= 0 && countMatchingGroup(fallingTile.col, landedRow) >= kMIN_GROUP_TO_CLEAR) {
        destroyGroup(fallingTile.col, landedRow);
    }

    return true;
}

// ========================================================
// Static methods
// ========================================================

float World::gridToWorldPosX(int col) {
    return static_cast<float>(col * kTILE_SIZE);
}

float World::gridToWorldPosY(int row) {
    return static_cast<float>(row * kTILE_SIZE) + World::s_yOffset;
}

int World::worldToGridPosX(float col) {
    return static_cast<int>(col / kTILE_SIZE);    
}

int World::worldToGridPosY(float row) {
    return static_cast<int>( (row - s_yOffset) / kTILE_SIZE);
}

float World::fallingTileWorldPosY(const FallingTile& fallingTile) {
    return static_cast<float>(fallingTile.startRow * kTILE_SIZE) + 
            fallingTile.fallOffset + World::s_yOffset;
}

bool World::inGridBounds(int col, int row) {
    return col >= 0 && col < kCOLS && row >= 0 && row < kROWS;
}

void World::shiftOffsetY(float dy) {
    World::s_yOffset += dy;
}

float World::getOffsetY() {
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

bool World::hitTile(int x, int y) {
    assert(inGridBounds(x, y));
    Tile& tile = m_grid[y][x];
    if (tile.type == TileType::TILE_EMPTY || --tile.numHits > 0) {
        return false;
    }

    destroyGroup(x, y);
    return true;
}

bool World::clearTilesForRespawn(int col, int row) {
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

void World::buildOneRow() {
    // replace every row with the row beneath it. skipping the last row
    for (int row = 0; row < kROWS - 1; ++row) {
        m_grid[row] = m_grid[row + 1];
    }

    // randomize the last row
    for (int col = 0; col < kCOLS; ++col) {
        const Tile tile = makeRandomTile();
        setTile(col, kROWS - 1, tile);
    }

    for (FallingTile& fallingTile : m_fallingTiles) {
        --fallingTile.startRow;
        --fallingTile.targetRow;
    }

    m_fallingTiles.erase(
        std::remove_if(m_fallingTiles.begin(), m_fallingTiles.end(),
                        [](const FallingTile& f) { return f.targetRow < 0; }),
        m_fallingTiles.end());
}

// ========================================================
// getters / setters
// ========================================================

void World::setTile(int col, int row, Tile type) {
    assert(inGridBounds(col, row));
    m_grid[row][col] = type;
}

const Tile& World::getTile(int col, int row) const {
    assert(inGridBounds(col, row));
    return m_grid[row][col];
}

const std::array<std::array<Tile, World::kCOLS>, World::kROWS>& World::getTiles() const { 
    return m_grid;
}

const std::vector<FallingTile>& World::getFallingTiles() const {
    return m_fallingTiles;
}

World::World() {
    generateInitialGrid();
}