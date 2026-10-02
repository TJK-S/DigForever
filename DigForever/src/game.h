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
    }

    const Player& getPlayer() const { return m_player; }
    const World& getWorld() const { return m_world; }
};

#endif