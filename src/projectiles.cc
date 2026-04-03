#include "projectiles.h"
#include <raylib.h>
#include <vector>

Projectile::Projectile() : position({0, 0}), direction({0, 0}), angle(0), is_shooting(false) {}

Projectile::Projectile(Vector2 pos) : position(pos), direction({0, 0}), angle(0), is_shooting(false) {}

ProjectilePool::ProjectilePool(int initial_size) {
    pool.resize(initial_size);
    for (int i = initial_size - 1; i >= 0; i--) {
        free_indices.push_back(i);
    }
}

int ProjectilePool::get_free_projectile() {
    if (free_indices.empty()) {
        // Grow the pool if needed
        int currentSize = pool.size();
        int newSize = currentSize * 2;
        if (newSize == 0) newSize = 128;
        pool.resize(newSize);
        for (int i = newSize - 1; i >= currentSize; i--) {
            free_indices.push_back(i);
        }
    }
    
    int index = free_indices.back();
    free_indices.pop_back();
    return index;
}

void ProjectilePool::allocate_projectile(int index) {
    pool[index].is_shooting = true;
}

void ProjectilePool::deallocate_projectile(int index) {
    pool[index].is_shooting = false;
    pool[index].position = {0, 0};
    free_indices.push_back(index);
}
