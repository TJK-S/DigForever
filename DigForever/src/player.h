// player.h

#ifndef PLAYER_H
#define PLAYER_H

#include "world.h"

#include <array> 

class Player {
public:
    static constexpr float kX_SIZE  { 24.f };
    static constexpr float kY_SIZE  { 24.f };

    static constexpr float kX_SPEED { 200.f };
    static constexpr float kY_SPEED { 350.f };
    static constexpr float kGRAVITY { 1500.f };
    static constexpr float kMAX_Y_VEL { 1000.f };

    enum class Direction { Up, Down, Left, Right };
    
private:
    static constexpr float kMAX_HEALTH { 20.f };
    static constexpr float kPICKUP_HEALTH_AMT { 5.f };
    static constexpr float kHEALTH_DECAY_PER_SECOND { 1.f };

private:
    float m_xPos;
    float m_yPos;

    float m_xVelocity     { 0.f };
    float m_yVelocity     { 0.f };
    float m_xRemainder    { 0.f };
    float m_yRemainder    { 0.f };

    float m_health        { kMAX_HEALTH };
    bool m_onGround       { false };
    bool m_digging        { false };
    bool m_alive          { true };

    Direction m_direction { Direction::Down };

private:
    // physics --------------------------------------------

    [[nodiscard]] bool collidesAt(float x, float y, const World& world) const noexcept;
    void moveX(float amount, const World& world) noexcept;
    void moveY(float amount, const World& world) noexcept;
    void applyGravity(float dt) noexcept;

public:
    Player() noexcept;

    void update(float dt, const World& world) noexcept;
    
    [[nodiscard]] static bool hasIntersection(
        float x1, float y1, float w1, float h1,
        float x2, float y2, float w2, float h2
    ) noexcept;

    // getters / setters ----------------------------------

    void  setVelocityX(float vel) noexcept;
    void  setVelocityY(float vel) noexcept;
    void  setFacing(Direction dir) noexcept;
    void  startDig() noexcept;
    void  stopDig() noexcept;
    void  shiftY(float dy) noexcept;
    void  setAlive(bool alive) noexcept;
    void  collectHealth(int numPickups) noexcept;

    [[nodiscard]] float getPosX() const noexcept;
    [[nodiscard]] float getPosY() const noexcept;
    [[nodiscard]] float getHealth() const noexcept;
    [[nodiscard]] bool  isOnGround() const noexcept;
    [[nodiscard]] bool  isDigging() const noexcept;
    [[nodiscard]] bool  isAlive() const noexcept;
    [[nodiscard]] Direction getDirection() const noexcept;
};

#endif