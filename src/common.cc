#include "common.h"
#include "config.h"
#include <cmath>
#include <cstdio>
#include <raylib.h>

void die(const char *message) {
  perror(message);
  exit(1);
}

// Convert grid position to absolute position
Vector2 GRID2SCREEN(float x, float y) {
  return {(float)CELL_OFFSET(x), (float)CELL_OFFSET(y)};
}

float get_angle_relative_to(Vector2 dest, Vector2 origin) {
  float angle = atan2(dest.y - origin.y, dest.x - origin.x);
  return (angle * RAD2DEG);
}

Vector2 get_world_mouse(Camera2D camera) {
  Vector2 mouse = GetMousePosition();
  return GetScreenToWorld2D(mouse, camera);
}
