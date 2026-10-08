// title.h

#ifndef TITLE_H
#define TITLE_H

#include "world.h"

#include <array>
#include <random>

class TitleScreen {
public:
    static constexpr int   kBG_COLS      { 20 }; // { Renderer::kVIRTUAL_SCREEN_WIDTH / World::kTILE_SIZE };
    static constexpr int   kBG_ROWS      { 13 }; // { Renderer::kVIRTUAL_SCREEN_HEIGHT / World::kTILE_SIZE  };
    static constexpr float kSCROLL_SPEED { 24.f };

private:
    std::array<std::array<TileType, kBG_COLS>, kBG_ROWS> m_background {};

    std::mt19937 m_rng { std::random_device{}() };
    std::discrete_distribution<int> m_tileDistr { World::kTILE_WEIGHTS.begin(), World::kTILE_WEIGHTS.end() };

    float m_scrollOffset   { 0.f };
    bool  m_startRequested { false };

private:
    void fillRow(int row) noexcept;
    void scrollOneRow() noexcept;

public:
    TitleScreen() noexcept;
    void update(float dt) noexcept;

    [[nodiscard]] TileType getBackgroundTile(int col, int row) const noexcept;
    [[nodiscard]] float getScrollOffset() const noexcept;
    [[nodiscard]] bool  startRequested() const noexcept;
};

#endif
