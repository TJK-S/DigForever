// world.h

#ifndef WORLD_H
#define WORLD_H

#include <random>
#include <array>
#include <vector>

enum class TileType {
    TILE_EMPTY,
    TILE_RED,
    TILE_GREEN,
    TILE_BLUE,
    TILE_YELLOW,

    TILE_TOUGH, // must always be the tile before the NUM_TILES for underlying type comparisons
    NUM_TILES
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
    static constexpr int    kCOLS         { 7 };
    static constexpr int    kROWS         { 15 };
    static constexpr int    kTILE_SIZE    { 32 };

    static constexpr int    kWORLD_WIDTH  { kCOLS * kTILE_SIZE };
    static constexpr int    kWORLD_HEIGHT { kROWS * kTILE_SIZE };

    static constexpr int    kCENTER_ROW   { 7 };
    static constexpr float  kSCROLL_LINE  { kCENTER_ROW * kTILE_SIZE };

    static constexpr int    kMIN_GROUP_TO_CLEAR { 4 }; // landing tiles kill groups of this size or larger
    
    struct GridPos { int x; int y; };
    
private:
    struct TileGroup {
        std::array<GridPos, kCOLS * kROWS> tiles {};
        int count { 0 };
    };

    std::array<std::array<Tile, kCOLS>, kROWS> m_grid;
    std::vector<FallingTile> m_fallingTiles;
    inline static float s_yOffset { 0.f };

    std::mt19937 m_rng { std::random_device{}() };
    std::uniform_int_distribution<int> m_tileDistr { 
        1, static_cast<int>(TileType::NUM_TILES) - 1}; // not including 0 which is the empty tile

private:
    // grid generation logic ------------------------------

    Tile makeRandomTile();
    void generateInitialGrid();

    // group logic ----------------------------------------

    TileGroup findGroup (int col, int row) const;
    int  countMatchingGroup(int col, int row) const;
    void destroyGroup(int x, int y);

    // floating logic -------------------------------------

    bool isReserved(int col, int row) const;
    bool isSupportedFromBelow(int col, int row, TileType groupType) const;
    bool groupIsFloating(int col, int row) const;

    // falling logic --------------------------------------

    int  findLandingRow(int col, int row) const;
    void startFallingTiles();
    bool targetIsUnsupported(const FallingTile& fallingTile);
    int  landTile(const FallingTile& fallingTile); // returns the row it landed in
    bool updateFallingTile(FallingTile& fallingTile, float dt, int& landedRow);

public:
    // static ---------------------------------------------

    static float gridToWorldPosX(int col);
    static float gridToWorldPosY(int row);
    static int   worldToGridPosX(float col);
    static int   worldToGridPosY(float row);
    static float fallingTileWorldPosY(const FallingTile& fallingTile);
    static bool  inGridBounds(int col, int row);
    static void  shiftOffsetY(float dy);
    static float getOffsetY();

    // exposed for game -----------------------------------

    std::vector<GridPos> updateFallingTiles(float dt); // returns vector of grid coordinates of recently landed tiles
    bool hitTile(int x, int y);
    bool clearTilesForRespawn(int col, int row);
    void buildOneRow();

    // getters / setters ----------------------------------

    void setTile(int x, int y, Tile type);

    const Tile& getTile(int x, int y) const;
    const std::array<std::array<Tile, kCOLS>, kROWS>& getTiles() const;
    const std::vector<FallingTile>& getFallingTiles() const;

    // class ----------------------------------------------

    World();
};

#endif