#include "common.h"
#include "draw.h"
#include "entities.h"
#include "wall.h"
#include <raylib.h>
#include <vector>

//TODO: Should not be in this file
void draw_arena(std::vector<Wall> &walls) {
  for (size_t i = 0; i < walls.size(); i++) {
    Vector2 position = walls[i].position;
    draw_wall(position, walls[i].type);
  }
}
