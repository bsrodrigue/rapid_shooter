#pragma once

#include <raylib.h>
#include <vector>

class Projectile {
public:
  Vector2 position;
  Vector2 direction;
  float angle;
  bool is_shooting;

  Projectile();
  Projectile(Vector2 position);
};

class ProjectilePool {
public:
  std::vector<Projectile> pool;
  std::vector<int> free_indices;

  ProjectilePool(int initial_size = 1000);

  int get_free_projectile();

  void allocate_projectile(int index);

  void deallocate_projectile(int index);
};
