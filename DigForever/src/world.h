// world.h

#ifndef WORLD_H
#define WORLD_H

#include <array>
#include <cassert>
#include <random>
#include <memory>

enum class Tile {
    TILE_EMPTY = 0,
    TILE_RED   = 1,
    TILE_GREEN = 2,
    TILE_BLUE  = 3,
    TILE_TOUGH = 4,

    NUM_TILES  = 5
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
    // could optimize this to be array of arrays
    // this way the build one row function isnt changing
    // litterally all kCOLS * kROWS
    // std::array<Tile, kROWS * kCOLS> m_grid;
    std::array<std::array<Tile, kCOLS>, kROWS> m_grid;
    inline static float s_yOffset { 0.f };

    std::random_device m_rd {};
    std::uniform_int_distribution<int> m_tileDistr { 
        1, static_cast<int>(Tile::NUM_TILES) - 1};

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

    Tile getTile(int x, int y) const {
        assert(inGridBounds(x, y));
        return m_grid[y][x];
    }

    void destroyMatchingTiles(int x, int y, Tile type) {
        if (!inGridBounds(x, y) || getTile(x, y) != type) {
            return;
        }

        setTile(x, y, Tile::TILE_EMPTY);

        destroyMatchingTiles(x + 1, y,     type);
        destroyMatchingTiles(x - 1, y,     type);
        destroyMatchingTiles(x,     y + 1, type);
        destroyMatchingTiles(x,     y - 1, type);
    }

    void generateInitialGrid() {
        for (int col = 0; col < kCOLS; ++col) {
            for (int row = 0; row < kROWS; ++row) {
                if (row <= kCENTER_ROW) {
                    setTile(col, row, Tile::TILE_EMPTY);
                } 
                else {
                    const Tile tile = static_cast<Tile>(m_tileDistr(m_rd));
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

        for (int col = 0; col < kCOLS; ++col) {
            // Not including 0 which is Empty
            // options are soft and hard
            setTile(col, kROWS - 1, static_cast<Tile>(m_tileDistr(m_rd)));
        }
    }
    const std::array<std::array<Tile, kCOLS>, kROWS>& getTiles() const { return m_grid; }
    
    World() {
        generateInitialGrid();
    }
};

#endif