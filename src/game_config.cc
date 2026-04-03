#include "game_config.h"
#include "cJSON.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

GameConfig current_config;

static std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static float get_float(cJSON* obj, const char* name, float default_val) {
    cJSON* item = cJSON_GetObjectItem(obj, name);
    return (item && cJSON_IsNumber(item)) ? (float)item->valuedouble : default_val;
}

static int get_int(cJSON* obj, const char* name, int default_val) {
    cJSON* item = cJSON_GetObjectItem(obj, name);
    return (item && cJSON_IsNumber(item)) ? (int)item->valueint : default_val;
}

static std::string get_string(cJSON* obj, const char* name, const std::string& default_val) {
    cJSON* item = cJSON_GetObjectItem(obj, name);
    return (item && cJSON_IsString(item)) ? item->valuestring : default_val;
}

GameConfig GameConfig::load_from_json(const std::string& filename) {
    std::string content = read_file(filename);
    if (content.empty()) return current_config;

    cJSON* root = cJSON_Parse(content.c_str());
    if (!root) {
        std::cerr << "Error parsing JSON in " << filename << std::endl;
        return current_config;
    }

    GameConfig config = current_config;

    cJSON* player_obj = cJSON_GetObjectItem(root, "player");
    if (player_obj) {
        config.player.bullet_damage = get_float(player_obj, "bullet_damage", 0.1f);
        config.player.shooting_interval = get_float(player_obj, "shooting_interval", 0.05f);
        config.player.max_health = get_float(player_obj, "max_health", 100.0f);
        config.player.speed = get_float(player_obj, "speed", 2.0f);
    }

    cJSON* projectiles_obj = cJSON_GetObjectItem(root, "projectiles");
    if (projectiles_obj) {
        config.projectiles.speed = get_float(projectiles_obj, "speed", 10.0f);
    }

    cJSON* enemies_obj = cJSON_GetObjectItem(root, "enemies");
    if (enemies_obj) {
        config.enemies.max_enemies = get_int(enemies_obj, "max_enemies", 100);
        config.enemies.collision_radius = get_float(enemies_obj, "collision_radius", 10.0f);
        config.enemies.vision_radius = get_float(enemies_obj, "vision_radius", 200.0f);
        config.enemies.shooting_interval = get_float(enemies_obj, "shooting_interval", 0.5f);
    }

    cJSON* window_obj = cJSON_GetObjectItem(root, "window");
    if (window_obj) {
        config.window.width = get_int(window_obj, "width", 1200);
        config.window.height = get_int(window_obj, "height", 600);
        config.window.fps = get_int(window_obj, "fps", 60);
        config.window.title = get_string(window_obj, "title", "Rapid Shooter");
    }

    cJSON_Delete(root);
    return config;
}
