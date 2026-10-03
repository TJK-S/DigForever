// world.h

#ifndef WORLD_H
#define WORLD_H

#include <array>
#include <cassert>
#include <random>
#include <memory>

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

class World {
public:
    static constexpr int   kCOLS         { 7 };
    static constexpr int   kROWS         { 15 };
    static constexpr int   kTILE_SIZE    { 32 };

    static constexpr int   kWORLD_WIDTH  { kCOLS * kTILE_SIZE };
    static constexpr int   kWORLD_HEIGHT { kROWS * kTILE_SIZE };

    static constexpr int   kCENTER_ROW   { 7 };
    static constexpr float kSCROLL_LINE  { kCENTER_ROW * kTILE_SIZE };

private:
    std::array<std::array<Tile, kCOLS>, kROWS> m_grid;
    inline static float s_yOffset { 0.f };

    std::mt19937 m_rng { std::random_device{}() };
    std::uniform_int_distribution<int> m_tileDistr { 
        1, static_cast<int>(TileType::NUM_TILES) - 1}; // not including 0 which is the empty tile

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
    }
    
    const std::array<std::array<Tile, kCOLS>, kROWS>& getTiles() const { return m_grid; }
    
    World() {
        generateInitialGrid();
    }
};

#endif