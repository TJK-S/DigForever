// render.cpp

#include "render.h"

#include <cmath>

Renderer::Renderer(const Game& game) : m_game(game) {
    const auto load = [this](TileType type, const char* name) {
        m_tileTextures[static_cast<int>(type) - 1] =
            ::LoadTexture(::TextFormat("DigForever/assets/Tile_%s.png", name));
    };

    load(TileType::TILE_RED,    "Red");
    load(TileType::TILE_GREEN,  "Green");
    load(TileType::TILE_BLUE,   "Blue");
    load(TileType::TILE_YELLOW, "Yellow");
    load(TileType::TILE_TOUGH,  "Tough");
    load(TileType::TILE_HEALTH, "Health");
}

Renderer::~Renderer() {
    for (const ::Texture2D& texture : m_tileTextures) {
        if (texture.id != 0) {
            ::UnloadTexture(texture);
        }
    }
}

const ::Texture2D& Renderer::getTexture2DFromTile(TileType type) const noexcept {
    return m_tileTextures[static_cast<int>(type) - 1];
}

// throws when no texture is found for a
void Renderer::renderWorld() noexcept {
    const World& world = m_game.getWorld();

    for (int col = 0; col < World::kCOLS; ++col) {
        for (int row = 0; row < World::kROWS; ++row) {
            const int xPos = col * World::kTILE_SIZE;
            const int yPos = row * World::kTILE_SIZE;

            const Tile& tile = world.getTile(col, row);

            constexpr ::Color kBackgroundColor { ::WHITE };
            ::DrawRectangle(
                kX_WORLD_OFFSET + xPos, kY_WORLD_OFFSET + static_cast<int>(world.getOffsetY()) + yPos,
                World::kTILE_SIZE, World::kTILE_SIZE, kBackgroundColor);
                
            if (tile.type == TileType::TILE_EMPTY) {                    
                continue;
            }

            ::DrawTexture(
                getTexture2DFromTile(tile.type), 
                kX_WORLD_OFFSET + xPos, kY_WORLD_OFFSET + static_cast<int>(world.getOffsetY()) + yPos,
                ::WHITE
            );
        }
    }

    for (const FallingTile& fallingTile : world.getFallingTiles()) {
        const float timer = fallingTile.fallTimer;

        constexpr int   kWobblePattern[] { 0, 1, 2, 1, 0, -1, -2, -1 };
        constexpr float kStepsPerSecond  { 30.f };

        const int step = static_cast<int>(timer * kStepsPerSecond) % 8;
        const int wobbleX = (timer >= 0.5f && timer < FallingTile::kTIME_BEFORE_FALL)
                            ? kWobblePattern[step] : 0;

        const int fallingTileY = static_cast<int>(world.fallingTileWorldPosY(fallingTile));

        ::DrawTexture(
            getTexture2DFromTile(fallingTile.tile.type),
            wobbleX + kX_WORLD_OFFSET + fallingTile.col * World::kTILE_SIZE,
            kY_WORLD_OFFSET + fallingTileY,
            ::WHITE
        );
    }
}

void Renderer::renderPlayer()  noexcept {
    const Player& player = m_game.getPlayer();

    ::DrawRectangle(
        kX_WORLD_OFFSET + static_cast<int>(player.getPosX()),
        kY_WORLD_OFFSET + static_cast<int>(player.getPosY()),
        static_cast<int>(Player::kX_SIZE), 
        static_cast<int>(Player::kY_SIZE),
        ::MAGENTA
    );
}

void Renderer::renderUI() noexcept {
    constexpr int kPaddingX { 10 };
    constexpr int kPaddingY { 20 };
    constexpr int kSpacingY { 40 };
    constexpr int kFontSize { 26 };
    constexpr ::Color kTextColor { ::GREEN }; 

    ::DrawText(
        ::TextFormat("Score: %d", m_game.getScore()),
        kWORLD_BORDER_RIGHT + kPaddingX, kPaddingY, kFontSize, kTextColor
    );

    ::DrawText(
        ::TextFormat("Depth: %d", m_game.getWorld().getDepth()),
        kWORLD_BORDER_RIGHT + kPaddingX, kPaddingY + kSpacingY * 1, kFontSize, kTextColor
    );

    ::DrawText(
        ::TextFormat("Health: %.0f", m_game.getPlayer().getHealth()),
        kWORLD_BORDER_RIGHT + kPaddingX, kPaddingY + kSpacingY * 2, kFontSize, kTextColor
    );
    
}
void Renderer::renderTitle(const TitleScreen& title) noexcept {
    // draw randomized tiles
    constexpr ::Color kBackgroundTint { 90, 90, 90, 255 };
    const int scroll = static_cast<int>(title.getScrollOffset());

    for (int row = 0; row < TitleScreen::kBG_ROWS; ++row) {
        for (int col = 0; col < TitleScreen::kBG_COLS; ++col) {
            ::DrawTexture(
                getTexture2DFromTile(title.getBackgroundTile(col, row)),
                col * World::kTILE_SIZE, 
                row * World::kTILE_SIZE - scroll,
                kBackgroundTint
            );
        }
    }

    // darken area behind the text
    constexpr int kRectY      { 60 };
    constexpr int kRectHeight { 240 };
    ::DrawRectangle(
        0, kRectY, 
        kVIRTUAL_SCREEN_WIDTH, 
        kRectHeight, 
        ::Fade(::BLACK, 0.6f));

    const auto drawCentered = [](const char* text, int y, int fontSize, ::Color color) {
        const int width = ::MeasureText(text, fontSize);
        ::DrawText(
            text, (kVIRTUAL_SCREEN_WIDTH - width) / 2, 
            y, fontSize, color);
    };

    constexpr const char* kTitle     { "DIG FOREVER" };
    constexpr int   kTitleFontSize   { 64 };
    constexpr int   kTitleY          { 80 };
    constexpr int   kShadowOffset    { 4 };

    drawCentered(kTitle, kTitleY + kShadowOffset, kTitleFontSize, ::DARKBROWN);
    drawCentered(kTitle, kTitleY, kTitleFontSize, ::GOLD);
    drawCentered("PRESS ENTER TO START", kTitleY + 120, 24, ::GREEN);
    drawCentered("WASD: Move / Aim    J: Dig    Esc: Quit", kTitleY + 180, 16, ::LIGHTGRAY);
}
