#pragma once

#include "framework/entities/entity.h"
#include "game/entityMesh.h"


class Weapon : public Entity {
public:

    enum WeaponType {
        DOUBLE_JUMPER
    };

    enum WeaponState {
        ON_GROUND,
        EQUIPPED
    };

    WeaponType type;
    WeaponState state = ON_GROUND;
    EntityMesh* visual = nullptr;
    Vector3 velocity = Vector3(0.0f, 0.0f, 0.0f);

    Weapon(WeaponType type, Mesh* mesh);

    void setOnGround(Vector3 position, Vector3 initial_velocity = Vector3(0.0f, 0.0f, 0.0f));
    void equip(Entity* owner, int slot);
    void update(float delta_time) override;
    void render(Camera* camera) override;
};