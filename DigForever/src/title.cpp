// title.cpp

#include "title.h"
#include <raylib.h>

TitleScreen::TitleScreen() noexcept {
    for (int row = 0; row < kBG_ROWS; ++row) {
        fillRow(row);
    }
}

void TitleScreen::fillRow(int row) noexcept {
    for (TileType& tile : m_background[row]) {
        tile = static_cast<TileType>(m_tileDistr(m_rng));
    }
}

void TitleScreen::scrollOneRow() noexcept {
    for (int row = 0; row < kBG_ROWS - 1; ++row) {
        m_background[row] = m_background[row + 1];
    }
    fillRow(kBG_ROWS - 1);
}

void TitleScreen::update(float dt) noexcept {

    m_scrollOffset += kSCROLL_SPEED * dt;
    while (m_scrollOffset >= World::kTILE_SIZE) {
        m_scrollOffset -= static_cast<float>(World::kTILE_SIZE);
        scrollOneRow();
    }

    if (::IsKeyPressed(KEY_ENTER)) {
        m_startRequested = true;
    }
}

TileType TitleScreen::getBackgroundTile(int col, int row) const noexcept {
    return m_background[row][col];
}

float TitleScreen::getScrollOffset() const noexcept { return m_scrollOffset; }
bool  TitleScreen::startRequested() const noexcept { return m_startRequested; }
