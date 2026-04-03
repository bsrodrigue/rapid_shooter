#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <raylib.h>
#include <vector>
#include "player.h"

void handle_game_input(Player& player, Camera2D& camera, std::vector<Vector2>& colliders);

#endif
