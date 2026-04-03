#pragma once

#include "editor_entities.h"
#include "gate.h"
#include "item.h"
#include "warpzone.h"
#include <raylib.h>
#include <vector>

#pragma once

#include "editor_entities.h"
#include "gate.h"
#include "item.h"
#include "warpzone.h"
#include <raylib.h>
#include <vector>
#include "enemy.h"

class EntityLoader : public EntityVisitor {
public:
  Vector2 position;

  EntityLoader(Vector2 position, std::vector<Wall> &walls,
               std::vector<Vector2> &wall_positions, std::vector<Gate> &gates,
               std::vector<Vector2> &gate_positions, std::vector<Enemy> &enemies,
               std::vector<Warpzone> &warpzones,
               std::vector<BaseItem> &items)
      : position(position), walls(walls), wall_positions(wall_positions),
        gates(gates), gate_positions(gate_positions), enemies(enemies),
        warpzones(warpzones), items(items) {}

  void visit(const EditorPlayer &player) override {
    // Handle player loading logic (e.g., set spawn point)
  }

  void visit(const EditorVoid &voidCell) override {
  }

  void visit(const EditorWall &wall) override {
    Wall w = (wall.type == UNBREAKABLE_WALL) ? create_ubreakable_wall(position)
                                             : create_breakable_wall(position);
    walls.push_back(w);
    wall_positions.push_back(w.position);
  }

  void visit(const EditorEnemy &enemy) override {
    Enemy e = create_enemy_from_level_data(position, enemy);
    enemies.push_back(e);
  }

  void visit(const EditorWarpzone &warpzone) override {
    warpzones.push_back({position, warpzone.destination});
  }

  void visit(const EditorItem &item) override {
    ItemTexture texture =
        (item.effect == HEALING_EFFECT) ? HEALING_CHIP_TEXTURE : KEY_TEXTURE;
    BaseItem i = create_base_item(item.effect, item.usage, texture, position);
    items.push_back(i);
  }

  void visit(const EditorGate &gate) override {
    Gate g;
    g.opened = false;
    g.position = position;
    g.type = BASE_GATE;
    gates.push_back(g);
    gate_positions.push_back(g.position);
  }

private:
  std::vector<Wall> &walls;
  std::vector<Vector2> &wall_positions;
  std::vector<Gate> &gates;
  std::vector<Vector2> &gate_positions;
  std::vector<Enemy> &enemies;
  std::vector<Warpzone> &warpzones;
  std::vector<BaseItem> &items;
};
