#include "weapon.h"

Weapon::Weapon(WeaponType type, Mesh* mesh)
    : type(type)
{
    Material material;

    visual = new EntityMesh(mesh, material);
    visual->is_collidable = false;

    addChild(visual);
}

void Weapon::setOnGround(Vector3 position)
{
    state = ON_GROUND;
    model.setTranslation(position);
}

AABB Weapon::getAABB()
{
    return visual->getAABB();
}