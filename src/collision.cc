#include "collision.h"
#include "config.h"
#include <raymath.h>

bool check_cell_collision(Vector2 cell_position, Vector2 position)
{
  Rectangle rect;

  rect.x = cell_position.x;
  rect.y = cell_position.y;

  rect.width = CELL_SIZE;
  rect.height = CELL_SIZE;

  return CheckCollisionPointRec(position, rect);
}

int check_cells_collision(std::vector<Vector2> cell_positions,
                          Vector2 position)
{

  for (size_t i = 0; i < cell_positions.size(); i++)
  {

    // const float distance = Vector2Distance(position, cell_positions[i]);

    // Skip if the distance is too far
    // if (distance > CELL_SIZE + 20)
    // {
    //   continue;
    // }

    if (check_cell_collision(cell_positions[i], position))
    {
      return i;
    }
  }

  return -1;
}
