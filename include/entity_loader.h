#pragma once
#include "editor_entities.h"
#include "game_state.h"
#include "gate.h"
#include "item.h"
#include <cmath>
#include <raylib.h>

// Loads entities from level data into game state
class EntityLoader : public EntityVisitor {
private:
  GameState &game_state;

public:
  Vector2 position;
  // Game State
  EntityLoader(GameState &gs) : game_state(gs) { position = Vector2{0, 0}; }

  void visit(const EditorPlayer &player) override {
    game_state.player.position = position;
  }

  void visit(const EditorVoid &voidCell) override {
    // Handle void cell loading logic
  }

  void visit(const EditorWall &wall) override {
    Wall w = (wall.type == UNBREAKABLE_WALL) ? create_ubreakable_wall(position)
                                             : create_breakable_wall(position);

    game_state.walls.push_back(w);
    game_state.wall_positions.push_back(w.position);
  }

  void visit(const EditorEnemy &enemy) override {
    Enemy e = create_enemy_from_level_data(position, enemy);
    game_state.enemies.push_back(e);
  }

  void visit(const EditorWarpzone &warpzone) override {
    game_state.warpzones.push_back(
        {.position = position, .destination = warpzone.destination});
  }

  void visit(const EditorItem &item) override {
    ItemTexture texture =
        (item.effect == HEALING_EFFECT) ? HEALING_CHIP_TEXTURE : KEY_TEXTURE;
    // TODO: Replace these with Factories
    BaseItem i = create_base_item(item.effect, item.usage, texture, position);
    game_state.items.push_back(i);
  }

  void visit(const EditorGate &gate) override {
    Gate g;
    g.opened = false;
    g.position = position;
    g.type = BASE_GATE;
    game_state.gates.push_back(g);
  }
};
