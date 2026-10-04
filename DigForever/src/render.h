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
            { TileType::TILE_RED,   ::LoadTexture(::TextFormat("%sRed.png",   assetPath)) }, 
            { TileType::TILE_GREEN, ::LoadTexture(::TextFormat("%sGreen.png", assetPath)) },
            { TileType::TILE_BLUE,  ::LoadTexture(::TextFormat("%sBlue.png",  assetPath)) },
            { TileType::TILE_TOUGH, ::LoadTexture(::TextFormat("%sTough.png", assetPath)) }
        };

        return tileTextures.at(tile);
    }
} // namespace

class Renderer {
public:
    static constexpr int kVIRTUAL_SCREEN_WIDTH  { 640 };
    static constexpr int kVIRTUAL_SCREEN_HEIGHT { 360 };

private:
    static constexpr int kX_OFFSET { 
        kVIRTUAL_SCREEN_WIDTH / 2 - (World::kCOLS * World::kTILE_SIZE) /2 };

    static constexpr int kY_OFFSET {
        -World::kTILE_SIZE * 3};

    const Game& m_game;
public:
    Renderer(const Game& game) : m_game(game) {}

    void renderWorld() {
        const World& world = m_game.getWorld();

        for (int col = 0; col < World::kCOLS; ++col) {
            for (int row = 0; row < World::kROWS; ++row) {
                const int xPos = col * World::kTILE_SIZE;
                const int yPos = row * World::kTILE_SIZE;

                const Tile& tile = world.getTile(col, row);
                if (tile.type == TileType::TILE_EMPTY) {
                    ::DrawRectangle(
                        kX_OFFSET + xPos, kY_OFFSET + static_cast<int>(world.getOffsetY()) + yPos,
                        World::kTILE_SIZE, World::kTILE_SIZE, ::WHITE);
                    
                    continue;
                }

                ::DrawTexture(
                    getTexture2DFromTile(tile.type), 
                    kX_OFFSET + xPos, kY_OFFSET + static_cast<int>(world.getOffsetY()) + yPos,
                    ::WHITE
                );
            }
        }  
    }

    void renderPlayer() {
        const Player& player = m_game.getPlayer();

        ::DrawRectangle(
            kX_OFFSET + static_cast<int>(player.getPosX()),
            kY_OFFSET + static_cast<int>(player.getPosY()),
            static_cast<int>(Player::kX_SIZE), 
            static_cast<int>(Player::kY_SIZE),
            ::MAGENTA
        );
    }
};

#endif