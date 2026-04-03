#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <string>

struct PlayerConfig {
    float bullet_damage;
    float shooting_interval;
    float max_health;
    float speed;
};

struct ProjectileConfig {
    float speed;
};

struct EnemyConfig {
    int max_enemies;
    float collision_radius;
    float vision_radius;
    float shooting_interval;
};

struct WindowConfig {
    int width;
    int height;
    int fps;
    std::string title;
};

struct GameConfig {
    PlayerConfig player;
    ProjectileConfig projectiles;
    EnemyConfig enemies;
    WindowConfig window;

    static GameConfig load_from_json(const std::string& filename);
};

extern GameConfig current_config;

#endif
