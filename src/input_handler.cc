#include "input_handler.h"
#include "common.h"
#include "game_config.h"

// Note: gate_positions and gates are global in main.cc.
// We might need to pass them to handle_game_input.

// Pre-existing variables from main.cc
extern float last_shot;
extern void player_shoot();

// Instead of global access, let's pass needed collections to a context struct
// But for now, I'll follow the user's style and keep it simple.

void handle_game_input(Player& player, Camera2D& camera, std::vector<Vector2>& colliders) {
    float now = GetTime();
    
    // Player Shooting
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && (now - last_shot) > current_config.player.shooting_interval) {
        player_shoot();
        last_shot = now;
    }

    // Movement
    player.handle_player_movement(colliders);
}
