#include "simple_test.h"
#include "game_world.h"
#include "projectiles.h"

TEST(ProjectilePoolAllocation) {
    ProjectilePool pool(10);
    
    int p1 = pool.get_free_projectile();
    ASSERT_EQ(p1, 0);
    pool.allocate_projectile(p1);
    
    int p2 = pool.get_free_projectile();
    ASSERT_EQ(p2, 1);
    pool.allocate_projectile(p2);
    
    pool.deallocate_projectile(p1);
    int p3 = pool.get_free_projectile();
    ASSERT_EQ(p3, 0); // Should reuse the last deallocated index
}

TEST(GameWorldInitialization) {
    GameWorld world;
    ASSERT_EQ(world.killCount, 0);
    ASSERT_EQ(world.walls.size(), 0);
}

int main() {
    TestRunner::getInstance().run();
    return 0;
}
