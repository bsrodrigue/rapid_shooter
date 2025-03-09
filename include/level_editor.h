#pragma once

#include "config.h"
#include "editor_entities.h"
#include "entities.h"
#include "item.h"
#include "wall.h"
#include <raylib.h>
#include <sys/types.h>

class LevelEditor {
public:
  const char *filename;

  // ------------------------[ Editor State ]------------------------------ //
  // General
  bool can_change_entity = true;
  bool placing_mode = false;

  // Inspection
  EditorGridCell *hovered_cell = nullptr;
  EditorGridCell *inspected_cell = nullptr;
  int inspected_cell_row = 0;
  int inspected_cell_col = 0;

  // Entity Dropdown
  bool entity_dropdown_is_open = false;
  EntityType current_entity = BWALL_ENTITY;

  // ----[Entities]
  // Enemies
  float enemy_health = 0.0f;
  float shooting_interval = 1.0f;
  float shooting_duration = 1.0f;
  float shooting_cooldown = 1.0f;
  bool tracks_player = false;
  bool follows_player = false;

  // Warpzone
  bool is_placing_warpzone_origin = true;
  Vector2 warpzone_origin_pos = {-1, -1};

  // ----[Items]
  ItemParams item_params{ItemEffect::NO_EFFECT, ItemUsage::NO_USAGE};

  // ----------------------------[ Grid ]---------------------------------- //
  EditorGridCell grid[CELL_COUNT][CELL_COUNT];

  LevelEditor() {}

  void init_editor();
  void load_level_data();
  void create_level_data();
  void save_level();

  Vector2 get_player_position();

  void handle_player_position_clearing();
  void place_entity(Vector2 mouse);
  void handle_entity_placement(Vector2 mouse, EntityType type);
};

void render_level_editor(Camera2D *camera);
void render_level_editor_ui(Camera2D *camera);
void handle_editor_input(Camera2D *camera);
void load_level_editor(const char *filename);
