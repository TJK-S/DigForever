// input.h

#ifndef INPUT_H
#define INPUT_H

#include "player.h"

class Input {
private:
    Player& m_player;
    bool m_prevDig { false };

public:
    Input(Player& player) noexcept;
    void handleInput() noexcept;
};

#endif