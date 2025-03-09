#include "config.h"
#include "editor_entities.h"
#include "level_editor.h"
#include "save.h"
#include <raylib.h>

void LevelEditor::save_level() { save_level_file_from_grid_data(this->filename, this->grid); }

void LevelEditor::load_level_data() {
  load_grid_data_from_level_file(this->filename, this->grid);
}

void LevelEditor::create_level_data() {
  // TODO: Get user input for level name and author.
  create_level_file(this->filename, "New Level", "Sawcekeeper");
}

Vector2 LevelEditor::get_player_position() {
  for (int y = 0; y < CELL_COUNT; y++) {
    for (int x = 0; x < CELL_COUNT; x++) {
      EntityType type = this->grid[y][x].type;
      if (type == PLAYER_ENTITY)
        return {(float)(x * CELL_SIZE), (float)(y * CELL_SIZE)};
    }
  }

  return {-1, -1};
}

void LevelEditor::handle_player_position_clearing() {
  Vector2 previous_pos = get_player_position();

  // Actually had an active position
  if (previous_pos.x != -1) {
    EditorGridCell *cell =
        &grid[MOUSE_TO_GRID(previous_pos.y)][MOUSE_TO_GRID(previous_pos.x)];

    // Clear position
    cell->type = EMPTY_ENTITY;
    cell->entity = EditorVoid{};
  }
}

void LevelEditor::handle_entity_placement(Vector2 mouse, EntityType type) {
  EditorGridCell *cell =
      &this->grid[MOUSE_TO_GRID(mouse.y)][MOUSE_TO_GRID(mouse.x)];

  cell->type = type;

  switch (cell->type) {
  case EMPTY_ENTITY:
    cell->entity = EditorVoid{};
    break;
  case PLAYER_ENTITY:
    cell->entity = EditorPlayer{};
    break;
  case WARPZONE_ENTITY:
    cell->entity = EditorWarpzone{mouse};
    break;
  case ITEM_ENTITY:
    cell->entity = item_params;
  case GATE_ENTITY:
    break;
  case UBWALL_ENTITY:
    cell->entity = EditorWall{UNBREAKABLE_WALL};
    break;
  case BWALL_ENTITY:
    cell->entity = EditorWall{BREAKABLE_WALL};
    break;
  case BASE_ENEMY_ENTITY:
    EditorEnemy enemy = EditorEnemy{};

    enemy.enemy_health = enemy_health;
    enemy.shooting_interval = shooting_interval;
    enemy.tracks_player = tracks_player;
    enemy.follows_player = follows_player;

    cell->entity = enemy;
    break;
  }
}

void LevelEditor::place_entity(Vector2 mouse) {
  // Check out of bounds
  if (MOUSE_TO_GRID(mouse.x) < 0 || MOUSE_TO_GRID(mouse.x) >= CELL_COUNT ||
      MOUSE_TO_GRID(mouse.y) < 0 || MOUSE_TO_GRID(mouse.y) >= CELL_COUNT) {
    return;
  }

  EntityType type = current_entity;

  if (type == PLAYER_ENTITY)
    handle_player_position_clearing();

  // Warpzones are more complex
  if (type == WARPZONE_ENTITY) {

    if (is_placing_warpzone_origin) {
      is_placing_warpzone_origin = false;
      can_change_entity = false;

      // Place destination before commiting

      warpzone_origin_pos = {(float)MOUSE_TO_GRID(mouse.x),
                             (float)MOUSE_TO_GRID(mouse.y)};

      TraceLog(LOG_INFO, "Warpzone Origin: %f, %f", warpzone_origin_pos.x,
               warpzone_origin_pos.y);
      return;
    }

    else {
      is_placing_warpzone_origin = true;
      can_change_entity = true;

      // Get cell from the warpzone origin position
      EditorGridCell *cell =
          &grid[(int)warpzone_origin_pos.y][(int)warpzone_origin_pos.x];

      TraceLog(LOG_INFO, "Warpzone Destination: %f, %f", MOUSE_TO_GRID(mouse.x),
               MOUSE_TO_GRID(mouse.y));

      cell->entity = EditorWarpzone{mouse};

      return;
    }
  }

  handle_entity_placement(mouse, type);
}
