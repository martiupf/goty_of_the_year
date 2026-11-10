#pragma once

#include "player.h"

class Collider {
public:
    ColliderType type;
    bool checkCollision(Player* player, Entity* map_entity);
};