// main.cpp

// As a convention i will use :: before anything included with Raylib

#include "raylib.h"

#include "game.h"
#include "render.h"
#include "title.h"

enum class GameState { Title, Playing };

namespace {
    constexpr float kVIRTUAL_W = static_cast<float>(Renderer::kVIRTUAL_SCREEN_WIDTH);
    constexpr float kVIRTUAL_H = static_cast<float>(Renderer::kVIRTUAL_SCREEN_HEIGHT);

    struct App {
        ::RenderTexture2D target;
        Game game;
        Renderer renderer { game };
        TitleScreen title;
        GameState screen { GameState::Title };
    };

    App* g_app = nullptr;

    void frame() {
        App& app = *g_app;
        
        // updates
        switch (app.screen) {
            case GameState::Title: {
                app.title.update(::GetFrameTime());
                if (app.title.startRequested()) { app.screen = GameState::Playing; }
                break;
            }
            case GameState::Playing: {
                app.game.update(::GetFrameTime());
                if (app.game.gameOverFinished()) {
                    app.game.reset();
                    app.title = TitleScreen{}; // clears startRequested so we don't jump straight back in
                    app.screen = GameState::Title;
                }
                break;
            }
        }

        // rendering
        ::BeginTextureMode(app.target);
        ::ClearBackground(::BLACK);
        switch (app.screen) {
            case GameState::Title:
                app.renderer.renderTitle(app.title);
                break;
            case GameState::Playing:
                app.renderer.renderWorld();
                app.renderer.renderPlayer();
                app.renderer.renderUI();
                break;
        }
        
        ::EndTextureMode();

        // scaling

        const float screenW = static_cast<float>(::GetScreenWidth());
        const float screenH = static_cast<float>(::GetScreenHeight());
        const float scale = std::min(screenW / kVIRTUAL_W, screenH / kVIRTUAL_H);

        const ::Rectangle sourceRec { 
            0.f, 0.f, 
            static_cast<float>(app.target.texture.width),
            -static_cast<float>(app.target.texture.height)
        };

        const ::Rectangle destRec {
            std::floor((screenW - kVIRTUAL_W * scale) * 0.5f),
            std::floor((screenH - kVIRTUAL_H * scale) * 0.5f),
            kVIRTUAL_W * scale,
            kVIRTUAL_H * scale
        };

        ::BeginDrawing();
        ::ClearBackground(::BLACK);
        ::DrawTexturePro(app.target.texture, sourceRec, destRec, ::Vector2{ 0.f, 0.f }, 0.f, ::WHITE);
        ::DrawFPS(10, 10);
        ::EndDrawing();
    }
} // namespace

int main() {
    ::SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    ::InitWindow(
        Renderer::kVIRTUAL_SCREEN_WIDTH, Renderer::kVIRTUAL_SCREEN_HEIGHT, "Dig Forever"
    );
    
    ::SetTargetFPS(60);
    
    {
        App app;
        app.target = ::LoadRenderTexture(
            Renderer::kVIRTUAL_SCREEN_WIDTH, Renderer::kVIRTUAL_SCREEN_HEIGHT
        );
        ::SetTextureFilter(app.target.texture, TEXTURE_FILTER_POINT);
        g_app = &app;

        while (!::WindowShouldClose()) {
            frame();
        }

        ::UnloadRenderTexture(app.target);
    }
    
    ::CloseWindow();
    return 0;
}