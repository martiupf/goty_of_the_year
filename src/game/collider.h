#pragma once

#include "player.h"

bool testAABBvsAABB(const AABB& a, const AABB& b);

class Collider {
public:
    ColliderType type;
    bool checkCollision(Player* player, Entity* map_entity);
};