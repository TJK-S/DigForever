// game.cpp

#include "game.h"

Game::Game() noexcept : m_input{ m_player } {}

void Game::update(float dt) {
    // freeze everything while "GAME OVER" is shown
    if (m_gameOver) {
        m_gameOverTimer += dt;
        return;
    }

    if (m_player.isAlive()) {
        m_input.handleInput();
    } 
    else {
        m_deathTimer += dt;
        
        if (m_deathTimer >= kMIN_DEAD_TIME) {
            const bool cleared = m_world.clearTilesForRespawn(
                World::worldToGridPosX(m_player.getPosX()), 
                World::worldToGridPosY(m_player.getPosY())
            );

            m_player.setAlive(cleared);
            m_deathTimer = (cleared) ? 0.f : m_deathTimer;
        }
    }

    m_player.update(dt, m_world);
    if (m_player.getHealth() <= 0.f) {
        m_gameOver = true;
        return;
    }

    const int healthCollected =
        m_world.collectHealthTiles(
            m_player.getPosX(), m_player.getPosY(), Player::kX_SIZE, Player::kY_SIZE) +
        m_world.collectFallingHealthTiles(
            m_player.getPosX(), m_player.getPosY(), Player::kX_SIZE, Player::kY_SIZE);
    
    if (healthCollected > 0) {
        m_player.collectHealth(healthCollected);
    }

    const std::vector<World::GridPos> landedTiles = m_world.updateFallingTiles(dt);        
    for (const World::GridPos& pos : landedTiles) {
        if (m_world.getTile(pos.x, pos.y).type == TileType::TILE_HEALTH) {
            continue;
        }

        const bool playerSmushed = Player::hasIntersection(
            m_player.getPosX(), m_player.getPosY(), 
            Player::kX_SIZE, Player::kY_SIZE,
            World::gridToWorldPosX(pos.x), World::gridToWorldPosY(pos.y), 
            World::kTILE_SIZE, World::kTILE_SIZE
        );

        if (!playerSmushed) { continue; }
        
        m_player.setAlive(false);
    }

    const float overshoot = m_player.getPosY() + Player::kY_SIZE - World::kSCROLL_LINE;
    if (overshoot > 0.f) {
        m_world.shiftOffsetY(-overshoot);
        m_player.shiftY(-overshoot);
        
        while (m_world.getOffsetY() <= -World::kTILE_SIZE) {
            m_world.shiftOffsetY(static_cast<float>(World::kTILE_SIZE));
            m_world.buildOneRow();
        }
    }
    
    m_score += m_world.getScoreThisFrame();
    if (!m_player.isDigging() || !m_player.isAlive()) { return; }
    
    int toRemoveX = World::worldToGridPosX(m_player.getPosX());
    int toRemoveY = World::worldToGridPosY(m_player.getPosY());
    
    const Player::Direction facing = m_player.getDirection();
    switch (facing) {
        case Player::Direction::Up:    toRemoveY -= 1; break;
        case Player::Direction::Down:  toRemoveY += 1; break;
        case Player::Direction::Left:  toRemoveX -= 1; break;
        case Player::Direction::Right: toRemoveX += 1; break;
    }

    if (!World::inGridBounds(toRemoveX, toRemoveY)) {
        return;
    }

    m_world.hitTile(toRemoveX, toRemoveY);
}

void Game::reset() {
    m_player = Player{};
    m_world  = World{};

    m_deathTimer    = 0.f;
    m_gameOver      = false;
    m_gameOverTimer = 0.f;
    m_score         = 0;
}