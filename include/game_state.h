#pragma once

#include "editor_entities.h"
#include "gate.h"
#include "level.h"
#include "player.h"
#include "warpzone.h"

class GameState {
public:
  // Player
  Player player;

  //  Entities
  std::vector<Wall> walls;
  std::vector<Vector2> wall_positions;
  std::vector<Gate> gates;
  std::vector<Vector2> gate_positions;
  std::vector<Enemy> enemies;
  std::vector<Warpzone> warpzones;
  std::vector<BaseItem> items;

  // Camera
  Camera2D camera;

  // Level
  Level current_level;

  GameState() {
    walls = std::vector<Wall>();
    wall_positions = std::vector<Vector2>();
    gates = std::vector<Gate>();
    gate_positions = std::vector<Vector2>();
    enemies = std::vector<Enemy>();
    warpzones = std::vector<Warpzone>();
    items = std::vector<BaseItem>();
  }

  void init_game_camera() {
    camera.target = player.position;
    camera.offset = {(float)WIN_WIDTH / 2, (float)WIN_HEIGHT / 2};
    camera.rotation = 0;
    camera.zoom = 2.0f;
  }
};
