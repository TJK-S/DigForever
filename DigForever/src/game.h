// game.h

#ifndef GAME_H
#define GAME_H

#include "player.h"
#include "world.h"
#include "input.h"

class Game {
private:
    Player m_player;    
    World  m_world;
    Input  m_input;

    float  m_deathTimer { 0.f };
    static constexpr float kMIN_DEAD_TIME { 5.f };

    bool   m_gameOver      { false };
    float  m_gameOverTimer { 0.f };
    static constexpr float kGAME_OVER_TIME { 3.f };

    int m_score { 0 };

public:
    Game() noexcept;
    
    void update(float dt);
    void reset();

    const Player& getPlayer() const { return m_player; }
    const World& getWorld() const { return m_world; }

    int  getScore() const { return m_score; }
    bool isGameOver() const { return m_gameOver; }
    bool gameOverFinished() const { return m_gameOver && m_gameOverTimer >= kGAME_OVER_TIME; }
};

#endif