// game.h

#include "player.h"
#include "world.h"
#include "input.h"

#ifndef GAME_H
#define GAME_H

class Game {
private:
    Player m_player;    
    World  m_world;
    Input  m_input;
public: 
    Game() : m_input{m_player} {}
    
    void update(float dt) {
        m_input.handleInput();
        m_player.update(dt, m_world.getTiles());

        if (!m_player.isDigging()) { return; }
        
        int toRemoveX = World::worldToGridPosX(m_player.getPosX());
        int toRemoveY = World::worldToGridPosY(m_player.getPosY());
        
        const Direction facing = m_player.getDirection();
        switch (facing) {
            case Direction::Up:    toRemoveY -= 1; break;
            case Direction::Down:  toRemoveY += 1; break;
            case Direction::Left:  toRemoveX -= 1; break;
            case Direction::Right: toRemoveX += 1; break;
        }

        m_world.setTile(toRemoveX, toRemoveY, Tile::TILE_EMPTY);
    }

    const Player& getPlayer() const { return m_player; }
    const World& getWorld() const { return m_world; }
};

#endif