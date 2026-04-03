#ifndef HUD_H
#define HUD_H

#include <raylib.h>
#include "player.h"

void draw_game_hud(const Player& player, int kill_count);
void draw_player_healthbar(const Player& player);
void draw_enemy_healthbar(Vector2 position, float health, float max_health);

#endif
