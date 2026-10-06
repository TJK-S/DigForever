// input.h

#include <raylib.h>

#include "player.h"

#ifndef INPUT_H
#define INPUT_H

class Input {
private:
    Player& m_player;
    bool m_prevDig { false };

public:
    Input(Player & player) : m_player(player) {}

    void handleInput() {

        const bool up    = ::IsKeyDown(KEY_W);
        const bool left  = ::IsKeyDown(KEY_A);
        const bool down  = ::IsKeyDown(KEY_S);
        const bool right = ::IsKeyDown(KEY_D);
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

        m_player.setFacing(facing);
        m_player.stopDig();
        if (dig && !m_prevDig) {
            m_player.startDig();
        }

        m_prevDig = dig;
    }
};

#endif