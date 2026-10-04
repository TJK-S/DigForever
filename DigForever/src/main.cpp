// main.cpp

// As a convention i will use :: before anything included with Raylib

#include "raylib.h"

#include "game.h"
#include "render.h"

int main() {
    ::SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    ::InitWindow(
        Renderer::kVIRTUAL_SCREEN_WIDTH, Renderer::kVIRTUAL_SCREEN_HEIGHT, "Dig Forever"
    );

    ::RenderTexture2D target = ::LoadRenderTexture(
        Renderer::kVIRTUAL_SCREEN_WIDTH, Renderer::kVIRTUAL_SCREEN_HEIGHT
    );
    
    ::SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
    ::SetTargetFPS(0);
    
    Game game;
    Renderer renderer { game };

    constexpr float kVIRTUAL_W = static_cast<float>(Renderer::kVIRTUAL_SCREEN_WIDTH);
    constexpr float kVIRTUAL_H = static_cast<float>(Renderer::kVIRTUAL_SCREEN_HEIGHT);

    while (!::WindowShouldClose()) {
        // update
        game.update(::GetFrameTime());
        
        // Draw

        ::BeginTextureMode(target);
        ::ClearBackground(::GRAY);
        renderer.renderWorld();
        renderer.renderPlayer();
        ::EndTextureMode();

        const float screenW = static_cast<float>(::GetScreenWidth());
        const float screenH = static_cast<float>(::GetScreenHeight());
        const float scale = std::min(screenW / kVIRTUAL_W, screenH / kVIRTUAL_H);

        const ::Rectangle sourceRec { 
            0.f, 0.f, 
            static_cast<float>(target.texture.width),
            -static_cast<float>(target.texture.height)
        };

        const ::Rectangle destRec {
            std::floor((screenW - kVIRTUAL_W * scale) * 0.5f),
            std::floor((screenH - kVIRTUAL_H * scale) * 0.5f),
            kVIRTUAL_W * scale,
            kVIRTUAL_H * scale
        };

        ::BeginDrawing();
        ::ClearBackground(::BLACK);
        ::DrawTexturePro(target.texture, sourceRec, destRec, ::Vector2{ 0.f, 0.f }, 0.f, ::WHITE);
        ::DrawFPS(10, 10);
        ::EndDrawing();
    }

    ::UnloadRenderTexture(target);
    ::CloseWindow();
    return 0;
}