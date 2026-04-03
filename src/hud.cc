#include "hud.h"
#include <cstdio>

void draw_player_healthbar(const Player& player) {
    float healthPercentage = (float)player.health / player.max_health;
    int rectWidth = 1 + (int)((32 - 1) * healthPercentage);

    Color color = GREEN;
    if (healthPercentage < 0.3) color = RED;
    else if (healthPercentage < 0.6) color = ORANGE;

    DrawRectangle((player.position.x - 16), (player.position.y - 25), rectWidth, 5, color);
}

void draw_enemy_healthbar(Vector2 position, float health, float max_health) {
    float healthPercentage = health / max_health;
    int rectWidth = 1 + (int)((32 - 1) * healthPercentage);

    Color color = GREEN;
    if (healthPercentage < 0.3) color = RED;
    else if (healthPercentage < 0.6) color = ORANGE;

    DrawRectangle((position.x - 16), (position.y - 25), rectWidth, 5, color);
}

void draw_game_hud(const Player& player, int kill_count) {
    char kills_text[64];
    sprintf(kills_text, "KILLS: %d", kill_count);
    DrawText(kills_text, 10, 10, 20, RAYWHITE);
    
    char health_text[64];
    sprintf(health_text, "HP: %.0f / %.0f", player.health, player.max_health);
    DrawText(health_text, 10, 40, 20, RAYWHITE);
}
