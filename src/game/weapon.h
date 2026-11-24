#pragma once

#include "framework/entities/entity.h"
#include "game/entityMesh.h"

class Weapon : public Entity {
public:

    enum WeaponType {
        MELEE,
        RANGED,
        MOVEMENT
    };

    enum WeaponState {
        ON_GROUND,
        EQUIPPED
    };

    WeaponType type;
    WeaponState state = ON_GROUND;

    EntityMesh* visual = nullptr;

    Weapon(WeaponType type, Mesh* mesh);

    void setOnGround(Vector3 position);
    void equip(Entity* owner, int slot);
    void render(Camera* camera) override;
};