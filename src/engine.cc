#include "engine.h"
#include "game_world.h"
#include "screen.h"
#include "rlImGui.h"
#include "textures.h"
#include "game_config.h"
#include "level_editor.h"
#include <ctime>
#include <cstring>

GameWorld game_world;
ScreenManager screen_manager;

void run_engine(const char* mode, const char* initial_level) {
    SetRandomSeed(time(0));

    InitWindow(WIN_WIDTH, WIN_HEIGHT, "Rapid Shooter");
    SetTargetFPS(FPS);
    load_textures();
    current_config = GameConfig::load_from_json("configs/game_config.json");
    rlImGuiSetup(true);

    if (strcmp(mode, "editor") == 0) {
        screen_manager.set_screen(LEVEL_EDITOR);
    } else {
        screen_manager.set_screen(GAME);
        if (initial_level) game_world.loadLevel(initial_level);
    }

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        if (screen_manager.active_screen == GAME) {
            game_world.update(dt);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (screen_manager.active_screen == GAME) {
            BeginMode2D(game_world.camera);
            game_world.render();
            EndMode2D();
        } else {
            render_level_editor(&game_world.camera);
            rlImGuiBegin();
            render_level_editor_ui(&game_world.camera);
            rlImGuiEnd();
        }

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
}
