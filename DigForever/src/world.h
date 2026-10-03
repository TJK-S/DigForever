// world.h

#ifndef WORLD_H
#define WORLD_H

#include <array>
#include <cassert>
#include <random>

enum class Tile {
    TILE_EMPTY = 0,
    TILE_SOFT  = 1,
    TILE_HARD  = 2,

    NUM_TILES  = 3
};

class World {
public:
    static constexpr int   kCOLS         { 7 };
    static constexpr int   kROWS         { 14 };
    static constexpr int   kTILE_SIZE    { 32 };

    static constexpr int   kWORLD_WIDTH  { kCOLS * kTILE_SIZE };
    static constexpr int   kWORLD_HEIGHT { kROWS * kTILE_SIZE };

    static constexpr int   kCENTER_ROW   { 5 };
    static constexpr float kSCROLL_LINE  { kCENTER_ROW * kTILE_SIZE };

private:
    // could optimize this to be array of arrays
    // this way the build one row function isnt changing
    // litterally all kCOLS * kROWS
    std::array<Tile, kROWS * kCOLS> m_grid;
    inline static float s_yOffset { 0.f };

public:
    static float gridToWorldPosX(float x) {
        return static_cast<float>(x * kTILE_SIZE);
    }

    static float gridToWorldPosY(float y) {
        return static_cast<float>(y * kTILE_SIZE) + World::s_yOffset;
    }

    static int worldToGridPosX(float x) {
        return static_cast<int>(x / kTILE_SIZE);
    }

    static int worldToGridPosY(float y) {
        return static_cast<int>(y / kTILE_SIZE);
    }

    static bool inGridBounds(int x, int y) {
        return x <= kCOLS && y <= kROWS;
    }

    static void shiftOffsetY(float dy) {
        World::s_yOffset += dy;
    }

    static void resetOffsetY() {
        World::s_yOffset = 0.f;
    }

    static float getOffsetY() {
        return World::s_yOffset;
    }

    void setTile(int x, int y, Tile type) {
        assert(inGridBounds(x, y));
        m_grid[x + kCOLS * y] = type;
    }

    Tile getTile(int x, int y) const {
        assert(inGridBounds(x, y));
        return m_grid[x + kCOLS * y];
    }

    void generateInitialGrid() {
        for (int col = 0; col < kCOLS; ++col) {
            for (int row = 0; row < kROWS; ++row) {
                if (row <= kCENTER_ROW) {
                    setTile(col, row, Tile::TILE_EMPTY);
                } 
                else {
                    setTile(col, row, Tile::TILE_SOFT);
                }
            }
        }
    }

    void buildOneRow() {
        std::random_device rd;
        std::mt19937 gen(rd());
 
        // replace every row with row beneath it. skipping the last row
        for (int col = 0; col < kCOLS; ++col) {
            for (int row = 0; row < kROWS - 1; ++row) {
                setTile(col, row, getTile(col, row + 1));
            }
        }

        // randomize the last row
        for (int col = 0; col < kCOLS; ++col) {
            // Not including 0 which is Empty
            // options are soft and hard
            std::uniform_int_distribution<int> distr(1, static_cast<int>(Tile::NUM_TILES) - 1);
            setTile(col, kROWS - 1, static_cast<Tile>(distr(gen)));
        }
    }
    const std::array<Tile, kROWS * kCOLS>& getTiles() const { return m_grid; }
    
    World() {
        generateInitialGrid();
    }
};

#endif