// world.h

#ifndef WORLD_H
#define WORLD_H

#include <random>
#include <array>
#include <vector>

// keep this order because of the kTILE_WEIGHTS
// might make an x table so dont have to make changes in multiple areas
enum class TileType {
    TILE_EMPTY,
    TILE_RED,
    TILE_ORANGE,
    TILE_YELLOW,
    TILE_GREEN,
    TILE_BLUE,
    TILE_PURPLE,

    // keep ordering of these last three
    TILE_TOUGH,  
    TILE_HEALTH,
    
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
    struct GridPos { int x; int y; };

    static constexpr int    kCOLS         { 7 };
    static constexpr int    kROWS         { 15 };
    static constexpr int    kTILE_SIZE    { 32 };

    static constexpr int    kWORLD_WIDTH  { kCOLS * kTILE_SIZE };
    static constexpr int    kWORLD_HEIGHT { kROWS * kTILE_SIZE };

    static constexpr int    kCENTER_ROW   { 9 };
    static constexpr float  kSCROLL_LINE  { kCENTER_ROW * kTILE_SIZE };

    static constexpr std::array<int, static_cast<int>(TileType::NUM_TILES)> kTILE_WEIGHTS {
        0,  // TILE_EMPTY  (never generated)
        10, // TILE_RED
        10, // TILE_ORANGE
        10, // TILE_YELLOW
        10, // TILE_GREEN
        10, // TILE_BLUE
        10, // TILE_PURPLE
        6,  // TILE_TOUGH
        1,  // TILE_HEALTH
    };

private:
    static constexpr int    kMIN_GROUP_TO_CLEAR { 4 }; // landing tiles kill groups of this size or larger
    static constexpr int    kNUM_HITS_TOUGH  { 5 };
    static constexpr int    kNUM_HITS_NORMAL { 1 };
    static constexpr int    kNUM_HITS_HEALTH { INT_MAX }; // unbreakable

    static constexpr int    kROWS_PER_SECTION   { 100 };
    static constexpr int    kROWS_BELOW_PLAYER { kROWS - 1 - kCENTER_ROW };

    static constexpr int    kNUM_COLORS         { static_cast<int>(TileType::TILE_TOUGH) - 1 }; // in enum class its everything before tough
    static constexpr int    kCOLORS_PER_SECTION { 3 };
    static constexpr int    kTILES_PER_SECTION  { kCOLORS_PER_SECTION + 2 }; // + tough + health

private:
    struct TileGroup {
        std::array<GridPos, kCOLS * kROWS> tiles {};
        int count { 0 };
    };

    std::array<std::array<Tile, kCOLS>, kROWS> m_grid;
    std::vector<FallingTile> m_fallingTiles;
    inline static float s_yOffset { 0.f };

    std::mt19937 m_rng { std::random_device{}() };
    std::discrete_distribution<int> m_tileDistr;
    std::array<TileType, kTILES_PER_SECTION> m_sectionTiles {};

    int m_scoreThisFrame   { 0 };
    int m_depth            { 0 }; // will say meters but can really be anything I want
    int m_bottomRowDepth   { kROWS_BELOW_PLAYER }; 
        
private:
    // grid generation logic ------------------------------

    [[nodiscard]] Tile makeRandomTile() noexcept;
    void pickSectionTiles();
    void generateInitialGrid() noexcept;
    void clearGrid() noexcept;

    // group logic ----------------------------------------

    [[nodiscard]] TileGroup findGroup (int col, int row) const noexcept;
    [[nodiscard]] int countMatchingGroup(int col, int row) const noexcept;
    void destroyGroup(int col, int row) noexcept;

    // floating logic -------------------------------------

    [[nodiscard]] bool isReserved(int col, int row) const noexcept;
    [[nodiscard]] bool isSupportedFromBelow(int col, int row, TileType groupType) const noexcept;
    [[nodiscard]] bool groupIsFloating(int col, int row) const noexcept;

    // falling logic --------------------------------------

    [[nodiscard]] int  findLandingRow(int col, int row) const noexcept;
    [[nodiscard]] int  landTile(const FallingTile& fallingTile) noexcept; // returns the row it landed in
    [[nodiscard]] bool targetIsUnsupported(const FallingTile& fallingTile) noexcept;
    [[nodiscard]] bool updateFallingTile(FallingTile& fallingTile, float dt, int& landedRow) noexcept;
    void startFallingTiles();

    // score calculation ----------------------------------

    void updateScore(int numDestroyedTile) noexcept;

public:
    // static ---------------------------------------------

    [[nodiscard]] static float gridToWorldPosX(int col) noexcept;
    [[nodiscard]] static float gridToWorldPosY(int row) noexcept;
    [[nodiscard]] static int   worldToGridPosX(float x) noexcept;
    [[nodiscard]] static int   worldToGridPosY(float y) noexcept;
    [[nodiscard]] static float fallingTileWorldPosY(const FallingTile& fallingTile) noexcept;
    [[nodiscard]] static bool  inGridBounds(int col, int row) noexcept;
    [[nodiscard]] static float getOffsetY() noexcept;
    
    static void  shiftOffsetY(float dy) noexcept;

    // exposed for game -----------------------------------

    [[nodiscard]] std::vector<GridPos> updateFallingTiles(float dt); // returns vector of grid coordinates of recently landed tiles
    [[nodiscard]] bool clearTilesForRespawn(int col, int row) noexcept;
    bool hitTile(int col, int row) noexcept;
    void buildOneRow();

    [[nodiscard]] int collectHealthTiles(float x, float y, float w, float h) noexcept;

    // getters / setters ----------------------------------

    void setTile(int col, int row, Tile type) noexcept;

    [[nodiscard]] const Tile& getTile(int col, int row) const noexcept;
    [[nodiscard]] const std::array<std::array<Tile, kCOLS>, kROWS>& getTiles() const noexcept;
    [[nodiscard]] const std::vector<FallingTile>& getFallingTiles() const noexcept;
    [[nodiscard]] int getScoreThisFrame() noexcept; 
    [[nodiscard]] int getDepth() const noexcept;

    // class ----------------------------------------------

    World();
};

#endif