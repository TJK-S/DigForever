// input.h

#ifndef INPUT_H
#define INPUT_H

#include "player.h"

class Input {
private:
    Player& m_player;
    float m_digCooldown { 0.f };
    static constexpr float kDIG_COOLDOWN { 0.2f };
    
public:
    Input(Player& player) noexcept;
    void handleInput(float dt) noexcept;
};

#endif
