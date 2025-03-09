#include <raylib.h>

#ifndef COMMON_H
#define COMMON_H
void die(const char *message);
float get_angle_relative_to(Vector2 dest, Vector2 origin);
Vector2 get_world_mouse(Camera2D camera);
Vector2 GRID2SCREEN(float x, float y);
#endif
