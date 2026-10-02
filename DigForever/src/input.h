// input.h

#include <raylib.h>

#include "player.h"

#ifndef INPUT_H
#define INPUT_H

class Input {
private:
    Player& m_player;

    bool prevDig { false };
public:
    Input(Player & player) : m_player(player) {}

    void handleInput() {

        const bool left  = ::IsKeyDown(KEY_A);
        const bool right = ::IsKeyDown(KEY_D);
        const bool jump  = ::IsKeyDown(KEY_SPACE);
        const bool dig   = ::IsKeyDown(KEY_J);

        m_player.setVelocityX(0.f);
        if (left && !right) {
            m_player.setVelocityX(-200.f);
        }

        if (!left && right) {
            m_player.setVelocityX(200.f);
        }

        if (jump && m_player.isOnGround()) {
            m_player.setVelocityY(-530.f);
        }

        prevDig = dig;
    }
};

#endif