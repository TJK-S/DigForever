// render.h

#include <raylib.h>

#include "game.h"

#ifndef RENDER_H
#define RENDER_H

class Renderer {
public:
    static constexpr int SCREEN_WIDTH  { 640 };
    static constexpr int SCREEN_HEIGHT { 360 };

private:
    static constexpr int X_OFFSET { 
        SCREEN_WIDTH / 2 - (World::kCOLS * World::kTILE_SIZE) /2 };

    static constexpr int Y_OFFSET {
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

                const Color color = [&](){
                    switch (world.getTile(col, row)) {
                        case Tile::TILE_SOFT: return ::GREEN;
                        case Tile::TILE_HARD: return ::RED;
                        default:              return ::WHITE;
                    }
                }();

                ::DrawRectangle(
                    X_OFFSET + xPos, Y_OFFSET + world.getOffsetY()+ yPos,
                    World::kTILE_SIZE, World::kTILE_SIZE, color);
            }
        }  
    }

    void renderPlayer() {
        const Player& player = m_game.getPlayer();

        ::DrawRectangle(
            X_OFFSET + player.getPosX(), Y_OFFSET + player.getPosY(),
            Player::kX_SIZE, Player::kY_SIZE, ::BLUE );
    }
};

#endif