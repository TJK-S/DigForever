// render.h

#ifndef RENDER_H
#define RENDER_H

#include <raylib.h>
#include <array>
#include "game.h"

class Renderer {
public:
    static constexpr int kVIRTUAL_SCREEN_WIDTH  { 640 };
    static constexpr int kVIRTUAL_SCREEN_HEIGHT { 360 };

private:
    static constexpr int kX_WORLD_OFFSET     { kVIRTUAL_SCREEN_WIDTH / 2 - (World::kCOLS * World::kTILE_SIZE) / 2 };
    static constexpr int kY_WORLD_OFFSET     { -World::kTILE_SIZE * 3 };
    static constexpr int kWORLD_BORDER_LEFT  { kX_WORLD_OFFSET };
    static constexpr int kWORLD_BORDER_RIGHT { kX_WORLD_OFFSET + (World::kCOLS * World::kTILE_SIZE)};

private:
    const Game& m_game;
    std::array<::Texture2D, static_cast<int>(TileType::NUM_TILES) - 1> m_tileTextures {}; // -1 because TILE_EMPTY has no texture

private:
    [[nodiscard]] const ::Texture2D& getTexture2DFromTile(TileType tile) const noexcept;

public:
    Renderer(const Game& game);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void renderWorld() noexcept;
    void renderPlayer() noexcept;
    void renderUI() noexcept;
};

#endif