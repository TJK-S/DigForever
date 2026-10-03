// main.cpp

// As a convention i will use :: before all Raylib functions/variables

#include "raylib.h"

#include "game.h"
#include "render.h"

int main() {
    ::InitWindow(Renderer::SCREEN_WIDTH, Renderer::SCREEN_HEIGHT, "Dig Forever");
    ::SetTargetFPS(0);
    
    Game game;
    Renderer renderer { game };

    while (!::WindowShouldClose()) {
        // update
        game.update(::GetFrameTime());
        
        // Draw
        ::BeginDrawing();
        ::ClearBackground(::GRAY);

        renderer.renderWorld();
        renderer.renderPlayer();
        
        ::DrawFPS(10, 10);
        ::EndDrawing();
    }

    ::CloseWindow();
    return 0;
}