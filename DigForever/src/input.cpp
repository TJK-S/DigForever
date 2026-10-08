// input.cpp

#include "input.h"
#include <raylib.h>

Input::Input(Player& player) noexcept : m_player(player) {}

void Input::handleInput() noexcept {
    const bool up    = ::IsKeyDown(KEY_W);
    const bool left  = ::IsKeyDown(KEY_A);
    const bool down  = ::IsKeyDown(KEY_S);
    const bool right = ::IsKeyDown(KEY_D);
    const bool jump  = ::IsKeyDown(KEY_SPACE);
    const bool dig   = ::IsKeyDown(KEY_J);

    Player::Direction facing = { Player::Direction::Down };
    m_player.setVelocityX(0.f);
    if (left && !right) {
        m_player.setVelocityX(-Player::kX_SPEED);
        facing = Player::Direction::Left;
    }

    if (!left && right) {
        m_player.setVelocityX(Player::kX_SPEED);
        facing = Player::Direction::Right;
    }

    if (up && !down) {
        facing = Player::Direction::Up;
    }
    else if (down && !up) {
        facing = Player::Direction::Down;
    }

    if (jump & m_player.isOnGround()) {
        m_player.setVelocityY(-Player::kY_SPEED);
    }

    m_player.setFacing(facing);
    m_player.stopDig();
    if (dig && !m_prevDig) {
        m_player.startDig();
    }

    m_prevDig = dig;
}