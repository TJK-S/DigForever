// render.h

#include <raylib.h>

#include "game.h"

#ifndef RENDER_H
#define RENDER_H

namespace {
    Color getTileColor(Tile tile) {
        switch (tile) {
            case Tile::TILE_EMPTY: return ::WHITE;
            case Tile::TILE_RED:   return ::RED;
            case Tile::TILE_GREEN: return ::GREEN;
            case Tile::TILE_BLUE:  return ::BLUE;
            case Tile::TILE_TOUGH: return ::BLACK;
            default:               return ::WHITE;
        }
        // in case the compiler complains
        return ::WHITE;
    }
} // namespace

class Renderer {
public:
    static constexpr int SCREEN_WIDTH  { 640 };
    static constexpr int SCREEN_HEIGHT { 360 };

private:
    static constexpr int kX_OFFSET { 
        SCREEN_WIDTH / 2 - (World::kCOLS * World::kTILE_SIZE) /2 };

    static constexpr int kY_OFFSET {
        -World::kTILE_SIZE};

    const Game& m_game;
public:
    Renderer(const Game& game) : m_game(game) {}
    void renderWorld() {
        const World& world = m_game.getWorld();

        for (int col = 0; col < World::kCOLS; ++col) {
            for (int row = 0; row < World::kROWS; ++row) {
                const int xPos = col * World::kTILE_SIZE;
                const int yPos = row * World::kTILE_SIZE;

                const Color color = getTileColor(world.getTile(col, row));

                ::DrawRectangle(
                    kX_OFFSET + xPos, kY_OFFSET + static_cast<int>(world.getOffsetY()) + yPos,
                    World::kTILE_SIZE, World::kTILE_SIZE, color);
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