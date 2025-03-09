#include "save.h"
#include "common.h"
#include "config.h"
#include "enemy.h"
#include "entities.h"
#include "json.h"
#include <functional>
#include <raylib.h>
#include <string.h>
#include <string>
#include <variant>

void create_level_file(const char *filename, const char *level_name,
                       const char *author_name) {

  const std::vector<std::string> entity_labels{
      "void", "walls", "players", "enemies", "items", "warpzones", "gates"};

  // New Save Format (JSON)
  cJSON *root = cJSON_CreateObject();

  for (const auto &label : entity_labels) {
    cJSON *items = cJSON_CreateArray();
    cJSON_AddItemToObject(root, label.c_str(), items);
  }

  char *json_str = cJSON_Print(root);
  bool saved = SaveFileData(filename, json_str, strlen(json_str));

  if (!saved) {
    TraceLog(LOG_ERROR, "An error occured while creating fresh level file");
  }

  cJSON_Delete(root);

  TraceLog(LOG_INFO, "Level File Created");
}

long get_file_size(const char *filename) {
  FILE *file = fopen(filename, "r"); // Open the file in read mode
  if (file == NULL) {
    printf("Error opening file: %s\n", filename);
    return -1; // Return -1 if the file could not be opened
  }

  fseek(file, 0, SEEK_END); // Move the file pointer to the end of the file
  long file_size = ftell(
      file); // Get the current file pointer position (which is the file size)
  fclose(file); // Close the file

  return file_size; // Return the size in bytes
}

template <typename T>
void parse_entities(cJSON *json_array, EditorGridCell (*grid)[100],
                    std::function<ParseResult(cJSON *)> parser) {
  if (cJSON_IsArray(json_array)) {
    int count = cJSON_GetArraySize(json_array);
    for (int i = 0; i < count; i++) {
      cJSON *item = cJSON_GetArrayItem(json_array, i);
      ParseResult result = parser(item);

      EditorGridCell &cell =
          grid[(int)result.position.y][(int)result.position.x];

      cell.entity = result.entity;
      cell.type = result.type;
    }
  }
}

void load_grid_data_from_level_file(const char *filename,
                                    EditorGridCell (*grid)[100]) {
  int data_size = get_file_size(filename);

  const char *loaded_data = (const char *)LoadFileData(filename, &data_size);

  if (loaded_data == NULL) {
    TraceLog(LOG_ERROR, "An error occured while loading level file");
    return;
  }

  cJSON *root = cJSON_Parse(loaded_data);

  if (root == NULL) {
    TraceLog(LOG_ERROR, "An error occured while loading level file");
    return;
  }

  cJSON *walls = cJSON_GetObjectItem(root, "walls");
  cJSON *players = cJSON_GetObjectItem(root, "players");
  cJSON *enemies = cJSON_GetObjectItem(root, "enemies");
  cJSON *items = cJSON_GetObjectItem(root, "items");
  cJSON *warpzones = cJSON_GetObjectItem(root, "warpzones");
  cJSON *gates = cJSON_GetObjectItem(root, "gates");

  // Parse each entity type
  parse_entities<EditorWall>(walls, grid, parse_editor_wall);
  parse_entities<EditorPlayer>(players, grid, parse_editor_player);
  parse_entities<EditorEnemy>(enemies, grid, parse_editor_enemy);
  parse_entities<EditorItem>(items, grid, parse_editor_item);
  parse_entities<EditorWarpzone>(warpzones, grid, parse_editor_warpzone);
  parse_entities<EditorGate>(gates, grid, parse_editor_gate);

  // Clean up
  cJSON_Delete(root);
  MemFree((void *)loaded_data); // Free the loaded data if needed
}

void save_level_file_from_grid_data(const char *filename,
                                    EditorGridCell (*grid)[100]) {
  cJSON *root = cJSON_CreateObject();

  cJSON *walls = cJSON_CreateArray();
  cJSON *players = cJSON_CreateArray();
  cJSON *enemies = cJSON_CreateArray();
  cJSON *items = cJSON_CreateArray();
  cJSON *warpzones = cJSON_CreateArray();
  cJSON *gates = cJSON_CreateArray();

  for (int i = 0; i < CELL_COUNT; i++) {
    for (int j = 0; j < CELL_COUNT; j++) {
      const EditorGridCell &cell = grid[i][j];

      // Assuming you have some way to identify the type of the entity
      if (std::holds_alternative<EditorWall>(cell.entity)) {
        cJSON *wall_json =
            serialize_editor_wall(std::get<EditorWall>(cell.entity), i, j);
        cJSON_AddItemToArray(walls, wall_json);
      } else if (std::holds_alternative<EditorPlayer>(cell.entity)) {
        cJSON *player_json =
            serialize_editor_player(std::get<EditorPlayer>(cell.entity), i, j);
        cJSON_AddItemToArray(players, player_json);
      } else if (std::holds_alternative<EditorEnemy>(cell.entity)) {
        cJSON *enemy_json =
            serialize_editor_enemy(std::get<EditorEnemy>(cell.entity), i, j);
        cJSON_AddItemToArray(enemies, enemy_json);
      } else if (std::holds_alternative<EditorItem>(cell.entity)) {
        cJSON *item_json =
            serialize_editor_item(std::get<EditorItem>(cell.entity), i, j);
        cJSON_AddItemToArray(items, item_json);
      } else if (std::holds_alternative<EditorWarpzone>(cell.entity)) {
        cJSON *warpzone_json = serialize_editor_warpzone(
            std::get<EditorWarpzone>(cell.entity), i, j);
        cJSON_AddItemToArray(warpzones, warpzone_json);
      } else if (std::holds_alternative<EditorGate>(cell.entity)) {
        cJSON *gate_json =
            serialize_editor_gate(std::get<EditorGate>(cell.entity), i, j);
        cJSON_AddItemToArray(gates, gate_json);
      }
    }
  }

  // Add arrays to root JSON object
  cJSON_AddItemToObject(root, "walls", walls);
  cJSON_AddItemToObject(root, "players", players);
  cJSON_AddItemToObject(root, "enemies", enemies);
  cJSON_AddItemToObject(root, "items", items);
  cJSON_AddItemToObject(root, "warpzones", warpzones);
  cJSON_AddItemToObject(root, "gates", gates);

  // Convert the JSON object to a string
  char *json_string = cJSON_Print(root);

  // Save the JSON string to a file
  bool saved =
      SaveFileData(filename, (unsigned char *)json_string, strlen(json_string));

  if (!saved) {
    TraceLog(LOG_ERROR, "An error occurred while saving level file");
  }

  TraceLog(LOG_INFO, "Level File Saved");

  // Cleanup
  cJSON_Delete(root);
  free(json_string);
}
