#include "game_world.h"
#include "draw.h"
#include "entity_loader.h"
#include "collision.h"
#include "shoot.h"
#include "hud.h"
#include "rlImGui.h"
#include "game.h"
#include <raymath.h>
#include <cstdio>
#include "config.h"
#include <algorithm>

GameWorld::GameWorld() : playerProjectiles(1000), enemyProjectiles(1000) {
    killCount = 0;
    lastShotTime = 0;
    
    camera.target = {0, 0};
    camera.offset = {(float)WIN_WIDTH / 2, (float)WIN_HEIGHT / 2};
    camera.rotation = 0;
    camera.zoom = 3;
}

GameWorld::~GameWorld() {}

void GameWorld::loadLevel(const std::string& filename) {
    currentLevel.filename = (char*)filename.c_str();
    currentLevel.load_level_data();
    
    walls.clear();
    wall_positions.clear();
    gates.clear();
    gate_positions.clear();
    enemies.clear();
    items.clear();
    warpzones.clear();
    spatialWallGrid.clear();
    spatialEnemyGrid.clear();
    
    EntityLoader loader({0, 0}, walls, wall_positions, gates, gate_positions,
                      enemies, warpzones, items);

    for (int y = 0; y < CELL_COUNT; y++) {
        for (int x = 0; x < CELL_COUNT; x++) {
            const EditorGridCell &cell = currentLevel.grid[y][x];
            loader.position = get_absolute_position_from_grid_position(x, y);
            std::visit([&](const Entity &entity) { entity.accept(loader); },
                     cell.entity);
        }
    }
    
    // Populate spatial grids once at load
    for (int i = 0; i < (int)wall_positions.size(); i++) {
        spatialWallGrid.add(wall_positions[i], i);
    }
    
    player.position = get_player_position_for_game(currentLevel.grid);
    camera.target = player.position;
    init_inventory(&inventory);
}

void GameWorld::update(float deltaTime) {
    camera.target = player.position;
    
    // Update enemy grid every frame since they move
    spatialEnemyGrid.clear();
    for (int i = 0; i < (int)enemies.size(); i++) {
        if (enemies[i].state != DEAD) {
            spatialEnemyGrid.add(enemies[i].position, i);
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float now = GetTime();
        if ((now - lastShotTime) > current_config.player.shooting_interval) {
            Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), camera);
            shoot_target(player.position, mouse, playerProjectiles);
            lastShotTime = now;
        }
    }
    
    std::vector<Vector2> colliders = wall_positions;
    for(const auto& gate : gates) {
        if (!gate.opened) colliders.push_back(gate.position);
    }
    player.handle_player_movement(colliders);
    player.angle = get_angle_relative_to(GetScreenToWorld2D(GetMousePosition(), camera), player.position);

    updateProjectiles(deltaTime);
    updateEnemies(deltaTime);
    checkCollisions();
    handleItemPicking();
}

void GameWorld::updateProjectiles(float deltaTime) {
    auto updatePool = [&](ProjectilePool& pool, bool isPlayer) {
        for (int i = 0; i < (int)pool.pool.size(); i++) {
            if (!pool.pool[i].is_shooting) continue;

            Projectile& p = pool.pool[i];
            
            if (isPlayer) {
                // Optimized Enemy Collision via Spatial Grid
                const auto* cell = spatialEnemyGrid.get_cell(p.position);
                if (cell) {
                    for (int enemyIdx : *cell) {
                        if (CheckCollisionCircles(p.position, 5, enemies[enemyIdx].position, 10)) {
                            enemies[enemyIdx].health -= current_config.player.bullet_damage;
                            if (enemies[enemyIdx].health <= 0) {
                                enemies[enemyIdx].state = DEAD;
                                killCount++;
                            }
                            pool.deallocate_projectile(i);
                            goto next_projectile;
                        }
                    }
                }
            } else {
                if (CheckCollisionCircles(p.position, 5, player.position, 10)) {
                    player.health -= 1.0f;
                    pool.deallocate_projectile(i);
                    goto next_projectile;
                }
            }

            // Optimized Wall Collision via Spatial Grid
            {
                const auto* wallCell = spatialWallGrid.get_cell(p.position);
                if (wallCell) {
                    for (int wallIdx : *wallCell) {
                        if (CheckCollisionPointRec(p.position, {wall_positions[wallIdx].x, wall_positions[wallIdx].y, (float)CELL_SIZE, (float)CELL_SIZE})) {
                            pool.deallocate_projectile(i);
                            goto next_projectile;
                        }
                    }
                }
            }

            p.position = Vector2Add(p.position, Vector2Multiply(p.direction, {current_config.projectiles.speed, current_config.projectiles.speed}));
            
            next_projectile:;
        }
    };

    updatePool(playerProjectiles, true);
    updatePool(enemyProjectiles, false);
}

void GameWorld::updateEnemies(float deltaTime) {
    for (auto& enemy : enemies) {
        if (enemy.state == DEAD) continue;

        bool is_in_range = CheckCollisionPointCircle(player.position, enemy.position, enemy.vision_radius);
        if (is_in_range) {
            if (enemy.tracks_player) {
                enemy.shooting_angle = get_angle_relative_to(player.position, enemy.position);
            }
            
            float now = GetTime();
            if ((now - enemy.last_shot_time) >= current_config.enemies.shooting_interval) {
                enemy.last_shot_time = now;
                if (enemy.tracks_player) {
                    shoot_target(enemy.position, player.position, enemyProjectiles);
                } else {
                    float angle = enemy.shooting_angle;
                    for (int j = 0; j < enemy.projectile_rate; j++) {
                        Vector2 dir = {cosf(angle * DEG2RAD), sinf(angle * DEG2RAD)};
                        shoot_straight(enemy.position, dir, enemyProjectiles);
                        angle += 10;
                    }
                    enemy.shooting_angle += 1;
                }
            }
        }

        if (enemy.can_move) {
            Vector2 next_position = Vector2MoveTowards(enemy.position, player.position, 2);
            if (check_wall_collision(wall_positions, next_position) == -1) {
                enemy.position = next_position;
            }
        }
    }
}

void GameWorld::checkCollisions() {
    for (const auto& wz : warpzones) {
        if (CheckCollisionPointCircle(player.position, {wz.position.x + 12.5f, wz.position.y + 12.5f}, 12.5f)) {
            player.position = wz.destination;
            camera.target = player.position;
        }
    }
}

void GameWorld::handleItemPicking() {
    for (int i = 0; i < (int)items.size(); i++) {
        if (!items[i].picked && CheckCollisionCircles(player.position, 10, items[i].position, 10)) {
            items[i].picked = true;
            if (items[i].usage == INSTANT_USAGE) {
                if (items[i].effect == HEALING_EFFECT) player.health = fminf(player.health + 10, player.max_health);
            } else {
                inventory_add_item(&inventory, items[i]);
            }
        }
    }
}

void GameWorld::render() {
    for (int y = 0; y < CELL_COUNT; y++) {
        for (int x = 0; x < CELL_COUNT; x++) {
            draw_floor(get_absolute_position_from_grid_position(x, y));
        }
    }

    draw_arena(walls);
    
    for (const auto& g : gates) {
        if (!g.opened) draw_warpzone(g.position);
    }
    
    for (const auto& wz : warpzones) {
        draw_warpzone(wz.position);
    }

    for (const auto& item : items) {
        if (!item.picked) {
            if (item.texture == HEALING_CHIP_TEXTURE) draw_healing_chip(item.position, 0);
            else if (item.texture == KEY_TEXTURE) draw_gate_key(item.position, 0);
        }
    }

    auto drawPool = [&](const ProjectilePool& pool) {
        for (const auto& p : pool.pool) {
            if (p.is_shooting) draw_projectile(p.position, p.angle);
        }
    };
    drawPool(playerProjectiles);
    drawPool(enemyProjectiles);

    for (const auto& enemy : enemies) {
        if (enemy.state != DEAD) {
            draw_enemy_healthbar(enemy.position, enemy.health, enemy.max_health);
            draw_base_enemy(enemy.position, enemy.shooting_angle);
        }
    }

    player.draw();
    
    draw_target_cursor(GetScreenToWorld2D(GetMousePosition(), camera), 0);
    draw_player_healthbar(player);
    draw_game_hud(player, killCount);
}
