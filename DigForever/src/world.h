// world.h

#ifndef WORLD_H
#define WORLD_H

#include <array>
#include <cassert>
#include <random>
#include <memory>
#include <vector>
#include <algorithm>

enum class TileType {
    TILE_EMPTY = 0,
    TILE_RED   = 1,
    TILE_GREEN = 2,
    TILE_BLUE  = 3,

    TILE_TOUGH = 4, // must always be the tile before the NUM_TILES for underlying type comparisons
    NUM_TILES  = 5
};

struct Tile {
    TileType type { TileType::TILE_EMPTY};
    int numHits   { 0 };
};

struct FallingTile {
    Tile tile;
    int col;
    int startRow;
    int targetRow;
    float fallOffset { 0.f };
    float fallTimer  { 0.f };

    static constexpr float kTIME_BEFORE_FALL { 2.f };
    static constexpr float kFALL_SPEED       { 128.f };
};

class World {
public:
    static constexpr size_t kCOLS         { 7 };
    static constexpr size_t kROWS         { 15 };
    static constexpr int    kTILE_SIZE    { 32 };

    static constexpr int    kWORLD_WIDTH  { static_cast<int>(kCOLS) * kTILE_SIZE };
    static constexpr int    kWORLD_HEIGHT { static_cast<int>(kROWS) * kTILE_SIZE };

    static constexpr int    kCENTER_ROW   { 7 };
    static constexpr float  kSCROLL_LINE  { kCENTER_ROW * kTILE_SIZE };

private:
    std::array<std::array<Tile, kCOLS>, kROWS> m_grid;
    std::vector<FallingTile> m_fallingTiles;
    inline static float s_yOffset { 0.f };

    std::mt19937 m_rng { std::random_device{}() };
    std::uniform_int_distribution<int> m_tileDistr { 
        1, static_cast<int>(TileType::NUM_TILES) - 1}; // not including 0 which is the empty tile

private:
    void destroyMatchingTiles(int x, int y, Tile tile) {
        if (!inGridBounds(x, y) || getTile(x, y).type != tile.type) {
            return;
        }

        setTile(x, y, Tile{});

        if (tile.type == TileType::TILE_TOUGH) {
            return; 
        }

        destroyMatchingTiles(x + 1, y,     tile);
        destroyMatchingTiles(x - 1, y,     tile);
        destroyMatchingTiles(x,     y + 1, tile);
        destroyMatchingTiles(x,     y - 1, tile);
    }

    Tile makeRandomTile() {
        const int randDistrNum = m_tileDistr(m_rng);
        const TileType type = static_cast<TileType>(randDistrNum);
        const int numHits = 
            (randDistrNum < static_cast<int>(TileType::TILE_TOUGH)) 
            ? 1 : 5;
            
        return Tile{ type, numHits };
    }

    void generateInitialGrid() {
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

    bool isReserved(int x, int y) const {
        for (const FallingTile& fallingTile : m_fallingTiles) {
            if (fallingTile.col == x && fallingTile.targetRow == y) {
                return true;
            }
        }

        return false;
    }

    bool hasMatchingNeigbor(int x, int y) const {
        const TileType type = m_grid[y][x].type;

        if ((inGridBounds(x + 1, y    ) && m_grid[y][x + 1].type == type) ||
            (inGridBounds(x - 1, y    ) && m_grid[y][x - 1].type == type) ||
            (inGridBounds(x,     y + 1) && m_grid[y + 1][x].type == type) ||
            (inGridBounds(x,     y - 1) && m_grid[y - 1][x].type == type)) { 
            
            return true; 
        }

        return false;
    }

    bool isFloating(int x, int y) const {
        if (y >= kROWS - 1) { return false; }

        return m_grid[y][x].type     != TileType::TILE_EMPTY &&
               m_grid[y][x].type     != TileType::TILE_TOUGH &&
               m_grid[y + 1][x].type == TileType::TILE_EMPTY &&
               !hasMatchingNeigbor(x, y);
    }

    int findLandingRow(int col, int row) {
        for (int r = row + 1; r < kROWS; ++r) {
            if (m_grid[r][col].type != TileType::TILE_EMPTY || isReserved(col, r)) {
                return r - 1;
            }
        }

        return kROWS - 1;
    }  
    
    void startFallingTiles() {
        for (int col = 0; col < kCOLS; ++col) {
            for (int row = kROWS - 2; row >= 0; --row) {
                if (!isFloating(col, row)) {
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

    void landTile(const FallingTile& fallingTile) {
        // Normally the target is empty because it was reserved
        // settle in the nearest empty cell above it instead of overwriting
        int row = fallingTile.targetRow;
        while (row >= 0 && m_grid[row][fallingTile.col].type != TileType::TILE_EMPTY) {
            --row;
        }
        if (row >= 0) {
            m_grid[row][fallingTile.col] = fallingTile.tile;
        }
    }

public:
    static float gridToWorldPosX(int x) {
        return static_cast<float>(x * kTILE_SIZE);
    }

    static float gridToWorldPosY(int y) {
        return static_cast<float>(y * kTILE_SIZE) + World::s_yOffset;
    }

    static int worldToGridPosX(float x) {
        return static_cast<int>(x / kTILE_SIZE);    
    }

    static int worldToGridPosY(float y) {
        return static_cast<int>( (y - s_yOffset) / kTILE_SIZE);
    }

    static float fallingTileWorldPosY(const FallingTile& fallingTile) {
        return static_cast<float>(fallingTile.startRow * kTILE_SIZE) + 
               fallingTile.fallOffset + World::s_yOffset;
    }

    static bool inGridBounds(int x, int y) {
        return x >= 0 && x < kCOLS && y >= 0 && y < kROWS;
    }

    static void shiftOffsetY(float dy) {
        World::s_yOffset += dy;
    }

    static float getOffsetY() {
        return World::s_yOffset;
    }

    void setTile(int x, int y, Tile type) {
        assert(inGridBounds(x, y));
        m_grid[y][x] = type;
    }

    const Tile& getTile(int x, int y) const {
        assert(inGridBounds(x, y));
        return m_grid[y][x];
    }

    bool hitTile(int x, int y) {
        assert(inGridBounds(x, y));
        Tile& tile = m_grid[y][x];
        if (tile.type == TileType::TILE_EMPTY || --tile.numHits > 0) {
            return false;
        }

        destroyMatchingTiles(x, y, tile);
        return true;
    }

    void updateFallingTiles(float dt) {
        startFallingTiles();
 
        for (std::size_t i = 0; i < m_fallingTiles.size();) {
            FallingTile& fallingTile = m_fallingTiles[i];

            fallingTile.fallTimer += dt;
            if (fallingTile.fallTimer < FallingTile::kTIME_BEFORE_FALL) {
                continue;
            }

            fallingTile.fallOffset += FallingTile::kFALL_SPEED * dt;
 
            const float landingOffset = 
                static_cast<float>((fallingTile.targetRow - fallingTile.startRow) * kTILE_SIZE);
            if (fallingTile.fallOffset >= landingOffset) {
                landTile(fallingTile);
                m_fallingTiles[i] = m_fallingTiles.back();
                m_fallingTiles.pop_back();
            }
            else {
                ++i;
            }
        }
    }

    void buildOneRow() {
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
    
    const std::array<std::array<Tile, kCOLS>, kROWS>& getTiles() const { return m_grid; }
    const std::vector<FallingTile>& getFallingTiles() const { return m_fallingTiles; }

    World() {
        generateInitialGrid();
    }
};

#endif