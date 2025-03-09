#pragma once

#include "entities.h"
#include "wall.h"
#include <raylib.h>

void draw_arena(std::vector<Wall> &walls);

Wall create_breakable_wall(Vector2 position);

Wall create_ubreakable_wall(Vector2 position);

void generate_walls(std::vector<Wall> &walls);
