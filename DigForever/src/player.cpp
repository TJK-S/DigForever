// player.cpp

#include "player.h"

#include <cmath>

// ========================================================
// Physics
// ========================================================

void Player::applyGravity(float dt) {
    m_yVelocity += kGRAVITY * dt;
}

bool Player::collidesAt(float x, float y, const World& world) const {
    // the right/bottom edges are exclusive, so step just inside them
    constexpr float kEdge = 0.01f;

    const int leftCol   = World::worldToGridPosX(x);
    const int rightCol  = World::worldToGridPosX(x + kX_SIZE - kEdge);
    const int topRow    = World::worldToGridPosY(y);
    const int bottomRow = World::worldToGridPosY(y + kY_SIZE - kEdge);

    for (int row = topRow; row <= bottomRow; ++row) {
        for (int col = leftCol; col <= rightCol; ++col) {
            if (!World::inGridBounds(col, row)) { continue; }
            
            const TileType type = world.getTile(col, row).type;

            if (type != TileType::TILE_EMPTY && type != TileType::TILE_HEALTH) {
                return true;
            }
        }
    }

    return false;
}

void Player::moveX(float amount, const World& world) {
    m_xRemainder += amount;
    int move = static_cast<int>(std::round(m_xRemainder));
    if (move == 0) { return; }

    m_xRemainder -= move;
    const int sign = (move > 0) ? 1 : -1;

    while (move != 0) {
        if (collidesAt(m_xPos + sign, m_yPos, world) || m_xPos + sign < 0.f || 
            m_xPos + sign + kX_SIZE > World::kWORLD_WIDTH) {

            m_xVelocity = 0.f;
            m_xRemainder = 0.f;
            break;
        }
        m_xPos += sign;
        move -= sign;
    }
}

void Player::moveY(float amount, const World& world) {
    m_yRemainder += amount;
    int move = static_cast<int>(std::round(m_yRemainder));
    if (move == 0) return;

    m_yRemainder -= move;
    const int sign = (move > 0) ? 1 : -1;
    m_onGround = false;
    
    while (move != 0) {
        if (collidesAt(m_xPos, m_yPos + sign, world) || m_yPos + sign < 0.f ||
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

void Player::update(float dt, const World& world) {        
    m_health -= kHEALTH_DECAY_PER_SECOND * dt;
    applyGravity(dt);

    moveX(m_xVelocity * dt, world);
    moveY(m_yVelocity * dt, world);
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
    void  Player::collectHealth(int numPickups) { 
        m_health = std::min(
            m_health + kPICKUP_HEALTH_AMT * static_cast<float>(numPickups), 
            kMAX_HEALTH
        );
    }

    float Player::getPosX()             const { return m_xPos; }
    float Player::getPosY()             const { return m_yPos; }
    float Player::getHealth()           const { return m_health; }
    bool  Player::isOnGround()          const { return m_onGround; }
    bool  Player::isDigging()           const { return m_digging; }
    bool  Player::isAlive()             const { return m_alive; } 


    Player::Direction Player::getDirection()    const { return m_direction; }