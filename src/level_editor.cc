#include "level_editor.h"
#include "camera.h"
#include "common.h"
#include "config.h"
#include "draw.h"
#include "entities.h"
#include "imgui.h"
#include "input_manager.h"
#include "level.h"
#include "raygui.h"
#include "save.h"
#include "wall.h"
#include <raylib.h>
#include <raymath.h>

// TODO: Define flexible level structure and editing
// TODO: Add undo action

LevelEditor level_editor;
InputManager editor_input_manager;

void load_level_editor(const char *filename) {
  level_editor.filename = filename;

  // TODO: Allow nesting
  if (!FileExists(filename)) {
    level_editor.create_level_data();
  }

  level_editor.load_level_data();
}

void render_entities() {
  for (int y = 0; y < CELL_COUNT; y++) {
    for (int x = 0; x < CELL_COUNT; x++) {

      // Fill entire space with floor texture beforehand
      draw_floor(GRID2SCREEN(x, y));

      EditorGridCell cell = level_editor.grid[y][x];
      draw_editor_entity(cell, GRID2SCREEN(x, y));
    }
  }
}

void handle_editor_actions(Camera2D *camera) {
  // Reset
  if (editor_input_manager.isActionPressed(GameAction::CANCEL)) {
    level_editor.placing_mode = false;
    level_editor.current_entity = EMPTY_ENTITY;
    level_editor.inspected_cell = nullptr;
  }

  if (IsKeyDown(KEY_LEFT_CONTROL)) {
    // Camera Zoom
    float mouse_scroll = GetMouseWheelMove();
    if (mouse_scroll != 0) {
      camera->zoom += mouse_scroll * 0.3f;
    }

    // Save
    // if (pressed_key == KEY_S) {
    //   level_editor.save_level();
    // }
  }

  if (level_editor.placing_mode) {
    // Shift for continuous placing
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
        (IsKeyDown(KEY_LEFT_SHIFT) && IsMouseButtonDown(MOUSE_BUTTON_LEFT))) {
      Vector2 mouse = get_world_mouse(*camera);

      level_editor.place_entity(mouse);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
      Vector2 mouse = get_world_mouse(*camera);

      EditorGridCell *cell =
          &level_editor.grid[MOUSE_TO_GRID(mouse.y)][MOUSE_TO_GRID(mouse.x)];

      // Clear
      cell->entity = EditorVoid{};
      cell->type = EMPTY_ENTITY;
    }
  }

  else {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 mouse = get_world_mouse(*camera);
      EditorGridCell *cell =
          &level_editor.grid[MOUSE_TO_GRID(mouse.y)][MOUSE_TO_GRID(mouse.x)];

      level_editor.inspected_cell =
          (cell->type != EMPTY_ENTITY) ? cell : nullptr;
      level_editor.inspected_cell_row = MOUSE_TO_GRID(mouse.y);
      level_editor.inspected_cell_col = MOUSE_TO_GRID(mouse.x);
    }
  }
}

// TODO: Setup a generic Key Manipulation Handler
void handle_editor_input(Camera2D *camera) {
  handle_editor_actions(camera);
  handle_camera_movement(camera);
}

std::string entity_types_str = get_enum_string();
std::string item_effect_str = get_item_effect_enum_string();

const char *dropdown_items = entity_types_str.c_str();
const char *item_effect_dropdown_items = item_effect_str.c_str();

bool item_effect_dropdown_is_open = false;

void render_level_editor(Camera2D *camera) {
  Vector2 mouse = get_world_mouse(*camera);

  render_entities();

  // TODO: Create utility function for this
  bool out_of_bounds =
      MOUSE_TO_GRID(mouse.x) < 0 || MOUSE_TO_GRID(mouse.x) >= CELL_COUNT ||
      MOUSE_TO_GRID(mouse.y) < 0 || MOUSE_TO_GRID(mouse.y) >= CELL_COUNT;

  if (level_editor.placing_mode) {
    render_mouse_hover_grid(level_editor.current_entity, mouse);
  }

  else if (!level_editor.placing_mode && !out_of_bounds) {
    // Inspect Entity
    EditorGridCell *cell =
        &level_editor.grid[MOUSE_TO_GRID(mouse.y)][MOUSE_TO_GRID(mouse.x)];

    level_editor.hovered_cell = (cell->type == EMPTY_ENTITY) ? nullptr : cell;
  }
}

static bool my_tool_active = true;

template <typename A> bool EnemyEditorUI(A *attributes_source) {
  return ImGui::InputFloat("Enemy Health", &attributes_source->enemy_health) ||
         ImGui::InputFloat("Shooting Interval",
                           &attributes_source->shooting_interval) ||
         ImGui::Checkbox("Tracks Player", &attributes_source->tracks_player) ||
         ImGui::Checkbox("Follows Player",
                         &attributes_source->follows_player) ||
         ImGui::InputFloat("Shooting Duration",
                           &attributes_source->shooting_duration) ||
         ImGui::InputFloat("Shooting Cooldown",
                           &attributes_source->shooting_cooldown);
}

void render_level_editor_ui(Camera2D *camera) {
  ImGui::SetNextWindowPos(ImVec2(WIN_WIDTH - 300, 0),
                          ImGuiCond_Once); // Top-left corner
  ImGui::SetNextWindowSize(ImVec2(300, WIN_HEIGHT),
                           ImGuiCond_Once); // 300x200 pixels
  ImGui::Begin("Level Editor", &my_tool_active,
               ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove);

  // Menu Bar
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save Level File")) {
        level_editor.save_level();
      }

      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  ImGui::Separator();

  // Current Position
  Vector2 mouse = get_world_mouse(*camera);
  ImGui::Text("Current Position: (%d, %d)", MOUSE_TO_GRID(mouse.x),
              MOUSE_TO_GRID(mouse.y));

  ImGui::Separator();

  // Current Hovered Cell
  ImGui::Text("Entity Type: %s",
              (level_editor.hovered_cell != nullptr)
                  ? get_entity_type_name(level_editor.hovered_cell->type)
                  : "Empty");

  ImGui::Separator();

  // Entity
  int current_entity = level_editor.current_entity;
  if (ImGui::Combo("Select Entity", &current_entity, dropdown_items, 8)) {
    level_editor.current_entity = (EntityType)current_entity;
    level_editor.placing_mode = true;
    level_editor.inspected_cell = nullptr;
  }

  //--- Entity Specific Parameters
  switch (level_editor.current_entity) {
  case BASE_ENEMY_ENTITY: {
    if (EnemyEditorUI(&level_editor)) {
      // TODO: Something?
    }
  } break;

  default:
    break;
  }

  //--- Inspected Entity
  if (level_editor.inspected_cell != nullptr) {
    ImGui::Text("Inspected Entity");
    ImGui::Text("Entity Type: %s",
                get_entity_type_name(level_editor.inspected_cell->type));

    EditorGridCell cell = *level_editor.inspected_cell;
    if (auto *enemy = std::get_if<EditorEnemy>(&cell.entity)) {
      if (EnemyEditorUI(enemy)) {
        TraceLog(LOG_INFO, "Update Enemy");
        level_editor
            .grid[level_editor.inspected_cell_row]
                 [level_editor.inspected_cell_col]
            .entity = *enemy;
      }
    }
  }

  ImGui::End();
}
