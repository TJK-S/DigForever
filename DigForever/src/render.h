// render.h

#include <raylib.h>

#include <unordered_map>

#include "game.h"

#ifndef RENDER_H
#define RENDER_H

namespace {
    ::Texture2D& getTexture2DFromTile(TileType tile) {
        const char* assetPath = "DigForever/assets/Tile_";

        static std::unordered_map<TileType, ::Texture2D> tileTextures {
            { TileType::TILE_RED,    ::LoadTexture(::TextFormat("%sRed.png",    assetPath)) }, 
            { TileType::TILE_GREEN,  ::LoadTexture(::TextFormat("%sGreen.png",  assetPath)) },
            { TileType::TILE_BLUE,   ::LoadTexture(::TextFormat("%sBlue.png",   assetPath)) },
            { TileType::TILE_YELLOW, ::LoadTexture(::TextFormat("%sYellow.png", assetPath)) },
            { TileType::TILE_TOUGH,  ::LoadTexture(::TextFormat("%sTough.png",  assetPath)) },
            { TileType::TILE_HEALTH, ::LoadTexture(::TextFormat("%sHealth.png", assetPath)) }
        };

        return tileTextures.at(tile);
    }
} // namespace

class Renderer {
public:
    static constexpr int kVIRTUAL_SCREEN_WIDTH  { 640 };
    static constexpr int kVIRTUAL_SCREEN_HEIGHT { 360 };

private:
    static constexpr int kX_WORLD_OFFSET     { kVIRTUAL_SCREEN_WIDTH / 2 - (World::kCOLS * World::kTILE_SIZE) / 2 };
    static constexpr int kY_WORLD_OFFSET     { -World::kTILE_SIZE * 3 };
    static constexpr int kWORLD_BORDER_LEFT  { kX_WORLD_OFFSET + (World::kCOLS * World::kTILE_SIZE)};
    static constexpr int kWORLD_BORDER_RIGHT { kX_WORLD_OFFSET };
    
    const Game& m_game;

public:
    Renderer(const Game& game) : m_game(game) {}

    void renderWorld() {
        constexpr ::Color kBackgroundColor { ::WHITE };

        const World& world = m_game.getWorld();

        for (int col = 0; col < World::kCOLS; ++col) {
            for (int row = 0; row < World::kROWS; ++row) {
                const int xPos = col * World::kTILE_SIZE;
                const int yPos = row * World::kTILE_SIZE;

                const Tile& tile = world.getTile(col, row);

                ::DrawRectangle(
                    kX_WORLD_OFFSET + xPos, kY_WORLD_OFFSET + static_cast<int>(world.getOffsetY()) + yPos,
                    World::kTILE_SIZE, World::kTILE_SIZE, kBackgroundColor);
                    
                if (tile.type == TileType::TILE_EMPTY) {                    
                    continue;
                }

                ::DrawTexture(
                    getTexture2DFromTile(tile.type), 
                    kX_WORLD_OFFSET + xPos, kY_WORLD_OFFSET + static_cast<int>(world.getOffsetY()) + yPos,
                    kBackgroundColor
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
                kBackgroundColor
            );
        }
    }

    void renderPlayer() {
        const Player& player = m_game.getPlayer();

        ::DrawRectangle(
            kX_WORLD_OFFSET + static_cast<int>(player.getPosX()),
            kY_WORLD_OFFSET + static_cast<int>(player.getPosY()),
            static_cast<int>(Player::kX_SIZE), 
            static_cast<int>(Player::kY_SIZE),
            ::MAGENTA
        );
    }

    void renderUI() {
        constexpr int kPaddingX { 10 };
        constexpr int kPaddingY { 20 };
        constexpr int kSpacingY { 40 };
        constexpr int kFontSIze { 26 };
        constexpr ::Color kTextColor { ::GREEN }; 

        ::DrawText(
            ::TextFormat("Score: %d", m_game.score),
            kWORLD_BORDER_LEFT + kPaddingX, kPaddingY, kFontSIze, kTextColor
        );

        ::DrawText(
            ::TextFormat("Depth: %d", m_game.getWorld().getDepth()),
            kWORLD_BORDER_LEFT + kPaddingX, kPaddingY + kSpacingY * 1, kFontSIze, kTextColor
        );

        ::DrawText(
            ::TextFormat("Health: %.0f", m_game.getPlayer().getHealth()),
            kWORLD_BORDER_LEFT + kPaddingX, kPaddingY + kSpacingY * 2, kFontSIze, kTextColor
        );
        
    }
};

#endif