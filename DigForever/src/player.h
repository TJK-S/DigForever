// player.h

#ifndef PLAYER_H
#define PLAYER_H

#include "world.h"

#include <array> 
#include <cmath>

enum class Direction {
    Up, Down, Left, Right
};

class Player {
public:
    static constexpr float kX_SIZE  { 32.f };
    static constexpr float kY_SIZE  { 32.f };

    static constexpr float kX_SPEED  { 200.f };
    static constexpr float kY_SPEED  { 530.f };
    static constexpr float k_GRAVITY { 1700.f };

private:
    float m_xPos;
    float m_yPos;

    float m_xVelocity     { 0.f };
    float m_xRemainder    { 0.f };

    float m_yVelocity     { 0.f };
    float m_yRemainder    { 0.f };

    bool m_onGround       { false };
    Direction m_direction { Direction::Down };
    bool m_digging        { false };

private:
    void applyGravity(float dt) {
        m_yVelocity += k_GRAVITY * dt;
    }   
    bool hasIntersection(
        float x1, float y1, float w1, float h1,
        float x2, float y2, float w2, float h2) const
    {
        return x1 < x2 + w2 && x1 + w1 > x2 &&
               y1 < y2 + h2 && y1 + h1 > y2;
    }

    bool collidesAt(float x, float y, const std::array<Tile, World::kCOLS * World::kROWS> tiles) const {
        for (int col = 0; col < World::kCOLS; ++col) {
            for (int row = 0; row < World::kROWS; ++row) {
                const Tile& tile = tiles[col + World::kCOLS * row];

                if (tile == Tile::TILE_EMPTY) { continue; }
            
                if (hasIntersection(
                    x, y, kX_SIZE, kX_SIZE,
                    World::gridToWorldPosX(col),
                    World::gridToWorldPosY(row),
                    World::kTILE_SIZE, World::kTILE_SIZE)) {
                        return true;
                }
            }
        }
        return false;
    }

    void moveX(float amount, const std::array<Tile, World::kCOLS * World::kROWS> tiles) {
        m_xRemainder += amount;
        int move = static_cast<int>(std::round(m_xRemainder));
        if (move == 0) { return; }

        m_xRemainder -= move;
        const int sign = (move > 0) ? 1 : -1;

        while (move != 0) {
            if (collidesAt(m_xPos + sign, m_yPos, tiles)) {
                m_xVelocity = 0.f;
                m_xRemainder = 0.f;
                break;
            }
            m_xPos += sign;
            move -= sign;
        }
    }

    void moveY(float amount, const std::array<Tile, World::kCOLS * World::kROWS> tiles) {
        m_yRemainder += amount;
        int move = static_cast<int>(std::round(m_yRemainder));
        if (move == 0) return;

        m_yRemainder -= move;
        const int sign = (move > 0) ? 1 : -1;
        m_onGround = false;
        
        while (move != 0) {
            if (collidesAt(m_xPos, m_yPos + sign, tiles)) {
                m_yVelocity = 0.f; 
                m_yRemainder = 0.f;
                m_onGround = (sign > 0);
                break;
            }
            m_yPos += sign;
            move -= sign;
        }
        
    }
public:
    Player() 
    :   m_xPos{static_cast<float>(World::kCOLS * World::kTILE_SIZE) * 0.5f},
        m_yPos{0.f}
    {}

    void update(float dt, const std::array<Tile, World::kCOLS * World::kROWS>& tiles) {        
        applyGravity(dt);

        moveX(m_xVelocity * dt, tiles);
        moveY(m_yVelocity * dt, tiles);
    }

    void  setVelocityX(float vel)  { m_xVelocity = vel; }
    void  setVelocityY(float vel)  { m_yVelocity = vel; }
    void  setFacing(Direction dir) { m_direction = dir; }
    void  startDig()               { m_digging = true; }
    void  stopDig()                { m_digging = false; }

    float getPosX()          const { return m_xPos; }
    float getPosY()          const { return m_yPos; }
    bool isOnGround()        const { return m_onGround; }
    Direction getDirection() const { return m_direction; }
    bool isDigging()         const { return m_digging; }
};

#endif