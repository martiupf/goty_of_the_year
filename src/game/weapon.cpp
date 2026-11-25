#include "weapon.h"
#include "world.h"
#include "player.h"
#include "collider.h"

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

void Weapon::setOnGround(Vector3 position, Vector3 initial_velocity)
{
    state = ON_GROUND;
    velocity = initial_velocity;
    model.setTranslation(position);
    if (visual) {
        visual->cast_shadows = true;
        visual->receive_blob_shadows = true;
    }
}

void Weapon::update(float delta_time)
{
    if (state != ON_GROUND) {
        Entity::update(delta_time);
        return;
    }

    if (model.getTranslation().y <= Player::DEADZONE_Y) {
        if (parent) {
            parent->removeChild(this);
        }
        delete this;
        return;
    }

    const float GRAVITY = 48.0f;
    const float MAX_FALL_SPEED = -24.0f;

    velocity.y -= GRAVITY * delta_time;
    if (velocity.y < MAX_FALL_SPEED) {
        velocity.y = MAX_FALL_SPEED;
    }

    auto collidesWithMap = [this]() -> bool {
        if (!World::instance || !World::instance->root) return false;
        AABB my_aabb = getAABB();
        for (Entity* map_entity : World::instance->root->children) {
            if (!map_entity || map_entity == this || !map_entity->is_collidable)
                continue;

            if (dynamic_cast<Player*>(map_entity) || dynamic_cast<Weapon*>(map_entity))
                continue;

            if (map_entity->collider_type == ColliderType::AABB_BOX && testAABBvsAABB(my_aabb, map_entity->getAABB()))
                return true;
        }
        return false;
        };

    // Colisión eje X
    Vector3 pos = model.getTranslation();
    model.setTranslation(pos + Vector3(velocity.x * delta_time, 0.0f, 0.0f));
    if (collidesWithMap()) {
        model.setTranslation(pos);
        velocity.x = 0.0f;
    }

    // Colisión eje Z
    pos = model.getTranslation();
    model.setTranslation(pos + Vector3(0.0f, 0.0f, velocity.z * delta_time));
    if (collidesWithMap()) {
        model.setTranslation(pos);
        velocity.z = 0.0f;
    }

    // Colisión eje Y
    pos = model.getTranslation();
    model.setTranslation(pos + Vector3(0.0f, velocity.y * delta_time, 0.0f));
    if (collidesWithMap()) {
        model.setTranslation(pos);
        if (velocity.y < 0.0f) {
            // Fricción al apoyar en el suelo para que frene tras deslizarse un poco
            float damping = std::min(1.0f, 10.0f * delta_time);
            velocity.x *= (1.0f - damping);
            velocity.z *= (1.0f - damping);
        }
        velocity.y = 0.0f;
    }

    Entity::update(delta_time);
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