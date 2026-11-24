#include "weapon.h"
#include "world.h"

Weapon::Weapon(WeaponType type, Mesh* mesh)
    : type(type)
{

    if (mesh) {
        local_aabb.min = mesh->box.center - mesh->box.halfsize;
        local_aabb.max = mesh->box.center + mesh->box.halfsize;
    }
    Material material;

    visual = new EntityMesh(mesh, material);
    visual->is_collidable = false;

    addChild(visual);
}

void Weapon::setOnGround(Vector3 position)
{
    state = ON_GROUND;
    model.setTranslation(position);
    if (visual) {
        visual->cast_shadows = true;
        visual->receive_blob_shadows = true;
    }
}

void Weapon::equip(Entity* owner, int slot)
{
    state = EQUIPPED;
    if (visual) {
        visual->cast_shadows = false;
        visual->receive_blob_shadows = false;
    }

    model.setTranslation(Vector3(0.0f, 0.0f, 0.0f));
    owner->addChild(this);
}

void Weapon::render(Camera* camera)
{
    if (state == EQUIPPED)
        return;

    Entity::render(camera);

    if (World::instance && World::instance->is_shadow_pass)
        return;

    AABB box = getAABB();

    Vector3 corners[8] = {
        Vector3(box.min.x, box.min.y, box.min.z),
        Vector3(box.max.x, box.min.y, box.min.z),
        Vector3(box.max.x, box.min.y, box.max.z),
        Vector3(box.min.x, box.min.y, box.max.z),
        Vector3(box.min.x, box.max.y, box.min.z),
        Vector3(box.max.x, box.max.y, box.min.z),
        Vector3(box.max.x, box.max.y, box.max.z),
        Vector3(box.min.x, box.max.y, box.max.z)
    };

    int indices[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.0f, 1.0f, 0.0f);

    glBegin(GL_LINES);
    for (int i = 0; i < 12; ++i) {
        glVertex3fv(&corners[indices[i][0]].x);
        glVertex3fv(&corners[indices[i][1]].x);
    }
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
}