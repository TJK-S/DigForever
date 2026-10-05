// player.h

#ifndef PLAYER_H
#define PLAYER_H

#include "world.h"

#include <array> 

class Player {
public:
    static constexpr float kX_SIZE  { 24.f };
    static constexpr float kY_SIZE  { 24.f };

    static constexpr float kX_SPEED  { 200.f };
    static constexpr float kY_SPEED  { 530.f };
    static constexpr float kGRAVITY { 1700.f };

    enum class Direction { Up, Down, Left, Right };

private:
    float m_xPos;
    float m_yPos;

    float m_xVelocity     { 0.f };
    float m_yVelocity     { 0.f };
    float m_xRemainder    { 0.f };
    float m_yRemainder    { 0.f };

    bool m_onGround       { false };
    bool m_digging        { false };
    bool m_alive          { true };

    Direction m_direction { Direction::Down };


private:
    // physics --------------------------------------------

    void applyGravity(float dt);
    bool collidesAt(
        float x, float y,
        const std::array<std::array<Tile, World::kCOLS>, World::kROWS>& tiles) const;
    
    void moveX(
        float amount, 
        const std::array<std::array<Tile, World::kCOLS>, World::kROWS>& tiles);

    void moveY(
        float amount,
        const std::array<std::array<Tile, World::kCOLS>, World::kROWS>& tiles);

public:
    Player();

    void update(
        float dt, 
        const std::array<std::array<Tile, World::kCOLS>, World::kROWS>& tiles);
    
    static bool hasIntersection(
        float x1, float y1, float w1, float h1,
        float x2, float y2, float w2, float h2);


    // getters / setters ----------------------------------
    void  setVelocityX(float vel);
    void  setVelocityY(float vel);
    void  setFacing(Direction dir);
    void  startDig();
    void  stopDig();
    void  shiftY(float dy);
    void  setAlive(bool alive);

    float getPosX()             const;
    float getPosY()             const;
    bool  isOnGround()          const;
    bool  isDigging()           const;
    bool  isAlive()             const;
    Direction getDirection()    const;
};

#endif