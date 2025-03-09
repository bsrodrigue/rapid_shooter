#include "collision.h"
#include "common.h"
#include "config.h"
#include "draw.h"
#include "enemy.h"
#include "entities.h"
#include "entity_loader.h"
#include "game.h"
#include "gate.h"
#include "imgui.h"
#include "inventory.h"
#include "item_drop.h"
#include "level.h"
#include "level_editor.h"
#include "player.h"
#include "projectiles.h"
#include "raylib.h"
#include "rlImGui.h"
#include "save.h"
#include "screen.h"
#include "shoot.h"
#include "textures.h"
// #include "shaders.h"
#include "wall.h"
#include "warpzone.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <string.h>
#include <variant>
#include <vector>

ScreenManager screen_manager;

Level level;

char *level_file;
char *game_mode;

// TODO: Maybe create a config file for player stats
float player_bullet_damage = 0.1;
float projectile_speed = 10;
float last_shot = 0;
float shooting_interval = 0.05;

// Experimentative Gameplay
int kill_count = 0;

static Camera2D camera;

static std::vector<Wall> walls;
static std::vector<Vector2> wall_positions;

static std::vector<Gate> gates;
static std::vector<Vector2> gate_positions;

static int enemy_count = 0;
std::array<Enemy, MAX_ENEMIES> enemies;

static int item_count = 0;
BaseItem items[MAX_ITEMS];

static int warpzone_count = 0;
Warpzone warpzones[10];

ProjectilePool player_projectiles;
ProjectilePool enemy_projectiles;

Player player;

Inventory inventory;

void player_shoot() {
  Vector2 mouse = get_world_mouse(camera);
  shoot_target(player.position, mouse, player_projectiles);
}

void load_entities() {
  EntityLoader loader({0, 0}, walls, wall_positions, gates, gate_positions,
                      enemies, enemy_count, warpzones, warpzone_count, items,
                      item_count);

  for (int y = 0; y < CELL_COUNT; y++) {
    for (int x = 0; x < CELL_COUNT; x++) {
      const EditorGridCell &cell = level.grid[y][x];
      loader.position = get_absolute_position_from_grid_position(x, y);

      std::visit([&](const Entity &entity) { entity.accept(loader); },
                 cell.entity);
    }
  }
}

void die(const char *message) {
  perror(message);
  exit(1);
}

void init_game_camera() {
  camera.target = player.position;
  camera.offset = {(float)WIN_WIDTH / 2, (float)WIN_HEIGHT / 2};
  camera.rotation = 0;
  camera.zoom = 3;
}

void init_window() {
  InitWindow(WIN_WIDTH, WIN_HEIGHT, "Rapid Shooter");

  if (!IsWindowReady())
    die("Failed to initialize window\n");

  SetTargetFPS(FPS);

  // HideCursor();
}

template <typename T>
int get_close_entity_index(const std::vector<T> &entities,
                           const Vector2 &position,
                           float proximity_radius = 20.0f,
                           float entity_radius = 12.5f) {
  int entity_index = -1;

  for (int i = 0; i < entities.size(); i++) {
    if (CheckCollisionPointCircle(
            position,
            {
                .x = entities[i].position.x + entity_radius,
                .y = entities[i].position.y + entity_radius,
            },
            proximity_radius)) {
      entity_index = i;
      break;
    }
  }

  return entity_index;
}

void handle_gate_opening() {
  int gate_index = get_close_entity_index(gates, player.position);

  if (gate_index == -1)
    return;

  if (IsKeyPressed(KEY_SPACE)) {

    int key_item_index = find_iventory_item_by_effect(inventory, KEY_EFFECT);

    if (key_item_index == -1 || inventory.items[key_item_index].used) {
      // Don't have a key? Fight some enemies, some of them drop keys...
      TraceLog(LOG_INFO, "You don't have a key");
      return;
    }

    gates[gate_index].opened = true;

    gate_positions.erase(gate_positions.begin() + gate_index);
    gates.erase(gates.begin() + gate_index);
  }
}

void handle_game_input(int pressed_key) {
  // Handle player shooting
  float now = GetTime();
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
      (now - last_shot) > shooting_interval) {
    player_shoot();
    last_shot = now;
  }

  // Dashing ?
  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
    // player.start_dash(get_world_mouse(camera));
  }

  std::vector<Vector2> colliders = wall_positions;

  colliders.insert(colliders.end(), gate_positions.begin(),
                   gate_positions.end());

  player.handle_player_movement(colliders);

  // Handle Gate opening logic
  handle_gate_opening();

  // Handle Warpzone logic
  int warpzone_index = -1;
  for (int i = 0; i < warpzone_count; i++) {
    if (CheckCollisionPointCircle(player.position,
                                  {
                                      .x = warpzones[i].position.x + 12.5f,
                                      .y = warpzones[i].position.y + 12.5f,
                                  },
                                  12.5f)) {
      warpzone_index = i;
      break;
    }
  }

  if (warpzone_index != -1) {
    const Vector2 destination = warpzones[warpzone_index].destination;

    player.position = destination;
  }
}

void handle_input(int pressed_key) {
  if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
    return;
  }

  switch (screen_manager.active_screen) {
  case UNKNOWN:
    break;
  case MENU:
    break;
  case GAME:
    handle_game_input(pressed_key);
    break;
  case LEVEL_EDITOR:
    handle_editor_input(&camera, pressed_key);
    break;
  }
}

template <typename E> void heal(E *entity, float health) {
  entity->health = (entity->health + health) >= entity->max_health
                       ? entity->max_health
                       : (entity->health + health);
}

void use_item(Player *player, ItemEffect effect) {
  switch (effect) {
  case HEALING_EFFECT:
    heal(player, 10);
    break;
  case PROJECTILE_BOOST_EFFECT:
    player_bullet_damage += 5;
    break;
  case SPECIAL_EFFECT:
    break;
  case NO_EFFECT:
    break;
  case KEY_EFFECT:
    break;
  }
}

void pick_item(int index, Player *player) {
  BaseItem item = items[index];

  if (item.usage == INSTANT_USAGE)
    use_item(player, item.effect);

  else {
    inventory_add_item(&inventory, item);
  }

  TraceLog(LOG_INFO, "Picked item %d", index);

  items[index].picked = true;
}

void handle_enemy_death(Enemy *enemy) {
  if (enemy->drops_items) {

    float drop_radius = 20.0f;

    // Drop all items
    for (int i = 0; i < enemy->item_drop.count; i++) {
      float drop_angle = GetRandomValue(0, 360);

      Vector2 drop_position = {
          .x = enemy->position.x + cosf(drop_angle * DEG2RAD) * drop_radius,
          .y = enemy->position.y + sinf(drop_angle * DEG2RAD) * drop_radius,
      };

      BaseItem item = drop(enemy->item_drop, enemy->position);
      item.position = drop_position;
      items[item_count++] = item;
    }
  }
}

void damage_enemy(int index) {
  if ((enemies[index].health - player_bullet_damage) <= 0) {
    enemies[index].state = DEAD;
    handle_enemy_death(&enemies[index]);

    // Experimentative Gameplay
    kill_count++;
    player_bullet_damage += 0.01;
  }

  enemies[index].health = enemies[index].health - player_bullet_damage;
}

void damage_wall(int index) {
  if ((walls[index].health - player_bullet_damage) <= 0) {
    // walls[index].state = DESTROYED;
    walls.erase(walls.begin() + index);
    wall_positions.erase(wall_positions.begin() + index);
  }

  walls.at(index).health = walls.at(index).health - player_bullet_damage;
}

void damage_player(float damage) {
  if ((player.health - damage) <= 0) {
    player.health = 0;
    // TODO: Implement Game Over (restart game);
    CloseWindow();
  }

  player.health -= damage;
}

int check_enemy_collision(Vector2 position, float radius) {
  for (int i = 0; i < enemy_count; i++) {
    if (enemies[i].state == DEAD)
      continue;

    if (CheckCollisionCircles(position, radius, enemies[i].position, 10))
      return i;
  }

  return -1;
}

int check_items_collision(Vector2 position, float radius) {
  for (int i = 0; i < item_count; i++) {
    if (items[i].picked)
      continue;

    if (CheckCollisionCircles(position, radius, items[i].position, 10))
      return i;
  }

  return -1;
}

void update_enemy_projectiles() {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (!enemy_projectiles.pool[i].is_shooting)
      continue;

    if (CheckCollisionCircles(enemy_projectiles.pool[i].position, 5,
                              player.position, 10)) {
      damage_player(1);
      enemy_projectiles.deallocate_projectile(i);
      continue;
    }

    // Find better way to check many-to-many collisions
    int touched = check_wall_collision(wall_positions,
                                       enemy_projectiles.pool[i].position);

    if (touched != -1) {
      switch (walls[touched].type) {
      case BREAKABLE_WALL:
        // Should enemies be able to destroy walls?
        damage_wall(touched);
        break;
      case UNBREAKABLE_WALL:
        break;
      }

      enemy_projectiles.pool[i].is_shooting = false;
      continue;
    }

    Vector2 projectile_pos =
        Vector2Add(enemy_projectiles.pool[i].position,
                   Vector2Multiply(enemy_projectiles.pool[i].direction,
                                   {projectile_speed, projectile_speed}));
    enemy_projectiles.pool[i].position = projectile_pos;
  }
}

void update_player_projectiles() {
  // TODO: Do you seriously want to loop over all the projectiles?
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (!player_projectiles.pool[i].is_shooting)
      continue;

    int enemy = check_enemy_collision(player_projectiles.pool[i].position, 5);

    if (enemy != -1) {
      damage_enemy(enemy);
      player_projectiles.deallocate_projectile(i);
      continue;
    }

    // Recycle in object pool for optimal memory usage
    // Find better way to check many-to-many collisions
    int touched = check_wall_collision(wall_positions,
                                       player_projectiles.pool[i].position);

    if (touched != -1) {
      switch (walls[touched].type) {
      case BREAKABLE_WALL:
        damage_wall(touched);
        break;
      case UNBREAKABLE_WALL:
        break;
      }
      player_projectiles.deallocate_projectile(i);
      continue;
    }

    Vector2 projectile_pos =
        Vector2Add(player_projectiles.pool[i].position,
                   Vector2Multiply(player_projectiles.pool[i].direction,
                                   {projectile_speed, projectile_speed}));
    player_projectiles.pool[i].position = projectile_pos;
  }
}

void handle_enemy_shoot(Enemy *enemy) {
  float time = GetTime();

  if (enemy->shooting_duration != 0) {
    if ((time - enemy->last_cooldown_time) < enemy->shooting_cooldown_duration)
      return;

    if (enemy->is_first_shot) {
      enemy->first_shot_time = time;
      enemy->is_first_shot = false;
    }

    else {
      // Shooting Duration Exceeded
      if ((time - enemy->first_shot_time) >= enemy->shooting_duration) {
        enemy->is_first_shot = true;
        enemy->last_cooldown_time = time;
        return;
      }
    }
  }

  if ((time - enemy->last_shot_time) >= enemy->shooting_interval) {
    enemy->last_shot_time = time;

    switch (enemy->type) {
    case BASE_ENEMY:
      if (enemy->tracks_player) {
        shoot_target(enemy->position, player.position, enemy_projectiles);
      }

      else {
        Vector2 direction;
        float shooting_angle = enemy->shooting_angle;

        // TODO: Extract this logic
        for (int i = 0; i < enemy->projectile_rate; i++) {

          direction = (Vector2){cosf(shooting_angle * DEG2RAD),
                                sinf(shooting_angle * DEG2RAD)};

          shoot_straight(enemy->position, direction, enemy_projectiles);

          shooting_angle += 10;
        }

        enemy->shooting_angle += 1;
      }
      break;
    }
  }
}

void handle_enemy_behaviour() {
  for (int i = 0; i < enemy_count; i++) {

    if (enemies[i].state == DEAD)
      continue;

    bool is_in_range = CheckCollisionPointCircle(
        player.position, enemies[i].position, enemies[i].vision_radius);

    if (is_in_range) {

      if (enemies[i].tracks_player) {
        enemies[i].shooting_angle =
            get_angle_relative_to(player.position, enemies[i].position);
      }

      handle_enemy_shoot(&enemies[i]);
    }

    if (enemies[i].can_move) {
      Vector2 next_position =
          Vector2MoveTowards(enemies[i].position, player.position, 2);

      if (check_wall_collision(wall_positions, next_position) == -1) {
        enemies[i].position = next_position;
      }
    }
  }
}

void update_positions() {
  camera.target = player.position;
  handle_enemy_behaviour();
  update_enemy_projectiles();
  update_player_projectiles();

  // Handle Item Picking
  int item_index = check_items_collision(player.position, 10);

  if (item_index != -1) {
    pick_item(item_index, &player);
  }
}

void update_player_angle() {
  Vector2 mouse = get_world_mouse(camera);
  player.angle = get_angle_relative_to(mouse, player.position);
}

void handle_updates() {
  switch (screen_manager.active_screen) {
  case UNKNOWN:
    break;
  case MENU:
    break;
  case GAME:
    update_positions();
    update_player_angle();
    break;
  case LEVEL_EDITOR:
    break;
  }
}

void draw_items() {
  for (int i = 0; i < MAX_ITEMS; i++) {
    if (!items[i].picked) {
      switch (items[i].texture) {
      case NO_TEXTURE:
        break;
      case HEALING_CHIP_TEXTURE:
        draw_healing_chip(items[i].position, 0);
        break;
      case KEY_TEXTURE:
        draw_gate_key(items[i].position, 0);
        break;
      }
    }
  }
}

void draw_gates() {
  for (int i = 0; i < gates.size(); i++) {
    if (!gates[i].opened) {
      draw_warpzone(gates[i].position);
    }
  }
}

void draw_projectiles(ProjectilePool projectile_pool) {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (projectile_pool.pool[i].is_shooting) {
      draw_projectile(projectile_pool.pool[i].position,
                      projectile_pool.pool[i].angle);
    }
  }
}

void draw_healthbar(Vector2 position, float max_health, float health, int width,
                    int height) {
  float health_percentage = health / max_health;
  int max_width = 32, min_width = 1;

  int rectWidth =
      min_width + (int)((max_width - min_width) * health_percentage);

  Color color = GREEN;

  if (health_percentage < 0.3) {
    color = RED;
  }

  else if (health_percentage < 0.6) {
    color = ORANGE;
  }

  DrawRectangle((position.x - (int)(width / 2)), (position.y - 25), rectWidth,
                5, color);
}

// TODO: Use this and think about creating maybe a universal utility for
// drawing entities
void draw_enemies() {
  for (int i = 0; i < enemy_count; i++) {
    if (enemies[i].state == DEAD)
      continue;

    switch (enemies[i].type) {
    case BASE_ENEMY: {
      Vector2 position = enemies[i].position;
      draw_healthbar(position, enemies[i].max_health, enemies[i].health, 32,
                     32);

      draw_base_enemy(enemies[i].position, enemies[i].shooting_angle);
    } break;
    }
  }
}

void render_floor() {
  for (int y = 0; y < CELL_COUNT; y++) {
    for (int x = 0; x < CELL_COUNT; x++) {
      draw_floor(get_absolute_position_from_grid_position(x, y));
    }
  }
}

void draw_player_target() {
  Vector2 mouse = get_world_mouse(camera);

  // TODO: Decide if player angle is needed.
  draw_target_cursor(mouse, 0);
}

void draw_player_healthbar(Player player) {
  // Calculate the width of the rectangle based on health
  float healthPercentage = (float)player.health / player.max_health;
  int rectWidth = 1 + (int)((32 - 1) * healthPercentage);

  Color color = GREEN;

  if (healthPercentage < 0.3) {
    color = RED;
  }

  else if (healthPercentage < 0.6) {
    color = ORANGE;
  }

  DrawRectangle((player.position.x - 16), (player.position.y - 25), rectWidth,
                5, color);
}

void draw_warpzones() {
  for (int i = 0; i < warpzone_count; i++) {
    draw_warpzone(warpzones[i].position);
  }
}

void render_game() {
  // Background
  render_floor();

  // Environment
  draw_arena(walls);

  // Interactibles
  draw_gates();
  draw_warpzones();
  draw_items();

  // Projectiles
  draw_projectiles(enemy_projectiles);
  draw_projectiles(player_projectiles);

  // Actors
  draw_enemies();
  player.draw();

  // HUD
  draw_player_target();
  draw_player_healthbar(player);
}

// TODO: Optimize level loading
void load_level() { load_entities(); }

void init_player() {
  player.position = get_player_position_for_game(level.grid);
}

void ScreenManager::init_game_screen() {
  level.filename = level_file;
  level.load_level_data();
  init_player();

  init_inventory(&inventory);
  load_level();
  init_game_camera();
}

void ScreenManager::init_level_editor_screen(const char *filename) {
  ShowCursor();
  camera.offset = {(float)WIN_WIDTH / 2, (float)WIN_HEIGHT / 2};
  camera.rotation = 0;
  camera.zoom = 1;
  load_level_editor(filename);
}

void ScreenManager::handle_screen_change() {
  if (!this->screen_changed())
    return;

  this->previous_screen = this->active_screen;

  switch (this->active_screen) {
  case UNKNOWN:
    break;
  case MENU:
    break;
  case GAME:
    this->init_game_screen();
    SetExitKey(KEY_ESCAPE);
    break;
  case LEVEL_EDITOR:
    this->init_level_editor_screen(level_file);
    SetExitKey(0);
    break;
  }
}

void set_initial_screen(const char *game_mode) {
  if (strcmp("editor", game_mode) == 0) {
    screen_manager.set_screen(LEVEL_EDITOR);
  }

  else if (strcmp("game", game_mode) == 0) {
    screen_manager.set_screen(GAME);
  }
}

bool show_message = false;

int main(int argc, char *argv[]) {

  if (argc != 3) {
    std::cout << "usage: autohacka [mode] [level_file]" << std::endl;
    std::cout << "possible modes: game, editor" << std::endl;
    return EXIT_FAILURE;
  }

  unsigned int seed = time(0);

  SetRandomSeed(seed);

  init_window();
  load_textures();
  // load_shaders();

  // Initialize ImGUI
  rlImGuiSetup(true);

  game_mode = argv[1];
  level_file = argv[2];

  set_initial_screen(game_mode);

  while (!WindowShouldClose()) {
    screen_manager.handle_screen_change();

    BeginDrawing();
    ClearBackground(BLACK);

    //================================================[Game
    // Camera]====================================================//

    BeginMode2D(camera);

    int pressed_key = GetKeyPressed();

    // Input
    handle_input(pressed_key);

    // State
    handle_updates();

    // UI
    if (screen_manager.active_screen == GAME) {
      render_game();
    }

    if (screen_manager.active_screen == LEVEL_EDITOR) {
      render_level_editor(&camera);
    }

    EndMode2D();

    //================================================[Overlay]====================================================//

    rlImGuiBegin();
    if (screen_manager.active_screen == GAME) {
    }

    if (screen_manager.active_screen == LEVEL_EDITOR) {
      render_level_editor_ui(&camera);
    }
    rlImGuiEnd();
    EndDrawing();
  }

  rlImGuiShutdown();

  CloseWindow();
  return 0;
}
