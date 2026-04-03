#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include "player.h"
#include "enemy.h"
#include "wall.h"
#include "gate.h"
#include "warpzone.h"
#include "item.h"
#include "projectiles.h"
#include "inventory.h"
#include "level.h"
#include "game_config.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

struct SpatialGrid {
    static constexpr int GRID_CELL_SIZE = 127; // Use prime-ish size
    std::unordered_map<int, std::vector<int>> cells;
    
    int get_cell_key(Vector2 pos) const {
        int x = (int)pos.x / GRID_CELL_SIZE;
        int y = (int)pos.y / GRID_CELL_SIZE;
        return x * 10000 + y;
    }
    
    void clear() { cells.clear(); }
    void add(Vector2 pos, int index) { cells[get_cell_key(pos)].push_back(index); }
    
    const std::vector<int>* get_cell(Vector2 pos) const {
        auto it = cells.find(get_cell_key(pos));
        return (it != cells.end()) ? &it->second : nullptr;
    }
};

class GameWorld {
public:
    GameWorld();
    ~GameWorld();

    void update(float deltaTime);
    void render();
    void loadLevel(const std::string& filename);

    // Encapsulated state
    Player player;
    Inventory inventory;
    std::vector<Wall> walls;
    std::vector<Vector2> wall_positions;
    std::vector<Gate> gates;
    std::vector<Vector2> gate_positions;
    std::vector<Enemy> enemies;
    std::vector<BaseItem> items;
    std::vector<Warpzone> warpzones;
    
    ProjectilePool playerProjectiles;
    ProjectilePool enemyProjectiles;
    
    SpatialGrid spatialWallGrid;
    SpatialGrid spatialEnemyGrid;

    Level currentLevel;
    int killCount = 0;
    float lastShotTime = 0;
    Camera2D camera;

private:
    void updateProjectiles(float deltaTime);
    void updateEnemies(float deltaTime);
    void checkCollisions();
    void handleItemPicking();
};

#endif
