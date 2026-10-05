// player.cpp

#include "player.h"

#include <cmath>

// ========================================================
// Physics
// ========================================================

void Player::applyGravity(float dt) {
    m_yVelocity += kGRAVITY * dt;
}

bool Player::collidesAt(float x, float y, const std::array<std::array<Tile, World::kCOLS>, World::kROWS>& tiles) const {
    for (int col = 0; col < World::kCOLS; ++col) {
        for (int row = 0; row < World::kROWS; ++row) {
            const Tile& tile = tiles[row][col];

            if (tile.type == TileType::TILE_EMPTY) { continue; }
        
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

void Player::moveX(float amount, const std::array<std::array<Tile, World::kCOLS>, World::kROWS>& tiles) {
    m_xRemainder += amount;
    int move = static_cast<int>(std::round(m_xRemainder));
    if (move == 0) { return; }

    m_xRemainder -= move;
    const int sign = (move > 0) ? 1 : -1;

    while (move != 0) {
        if (collidesAt(m_xPos + sign, m_yPos, tiles) || m_xPos + sign < 0.f || 
            m_xPos + sign + kX_SIZE > World::kWORLD_WIDTH) {

            m_xVelocity = 0.f;
            m_xRemainder = 0.f;
            break;
        }
        m_xPos += sign;
        move -= sign;
    }
}

void Player::moveY(float amount, const std::array<std::array<Tile, World::kCOLS>, World::kROWS>& tiles) {
    m_yRemainder += amount;
    int move = static_cast<int>(std::round(m_yRemainder));
    if (move == 0) return;

    m_yRemainder -= move;
    const int sign = (move > 0) ? 1 : -1;
    m_onGround = false;
    
    while (move != 0) {
        if (collidesAt(m_xPos, m_yPos + sign, tiles) || m_yPos + sign < 0.f ||
            m_yPos + sign + kY_SIZE > World::kWORLD_HEIGHT) {
            m_yVelocity = 0.f; 
            m_yRemainder = 0.f;
            m_onGround = (sign > 0);
            break;
        }
        m_yPos += sign;
        move -= sign;
    }
}

// ========================================================
// Interface
// ========================================================

Player::Player() 
:   m_xPos{static_cast<float>(World::kCOLS * World::kTILE_SIZE) * 0.5f},
    m_yPos{World::kSCROLL_LINE + (World::kTILE_SIZE - Player::kY_SIZE)}
{}

void Player::update(float dt, const std::array<std::array<Tile, World::kCOLS>, World::kROWS>& tiles) {        
    applyGravity(dt);

    moveX(m_xVelocity * dt, tiles);
    moveY(m_yVelocity * dt, tiles);
}

bool Player::hasIntersection(
    float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2)
{
    return x1 < x2 + w2 && x1 + w1 > x2 &&
            y1 < y2 + h2 && y1 + h1 > y2;
}


    void  Player::setVelocityX(float vel)  { m_xVelocity = vel; }
    void  Player::setVelocityY(float vel)  { m_yVelocity = vel; }
    void  Player::setFacing(Direction dir) { m_direction = dir; }
    void  Player::startDig()               { m_digging = true; }
    void  Player::stopDig()                { m_digging = false; }
    void  Player::shiftY(float dy)         { m_yPos += dy; }
    void  Player::setAlive(bool alive)     { m_alive = alive; }


    float Player::getPosX()             const { return m_xPos; }
    float Player::getPosY()             const { return m_yPos; }
    bool  Player::isOnGround()          const { return m_onGround; }
    bool  Player::isDigging()           const { return m_digging; }
    bool  Player::isAlive()             const { return m_alive; } 
    Player::Direction Player::getDirection()    const { return m_direction; }