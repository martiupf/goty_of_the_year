#include "player.h"
#include "collider.h"
#include "world.h"
#include "game.h"
#include <cmath>

Player::Player(Vector3 start_position, Mesh* mesh)
{
    
    model.setTranslation(start_position);
    
    velocity = 5.0f;
    rotation = 0.0f;

    vertical_velocity = 0.0f;
    gravity = 28.0f;
    is_grounded = false;

    is_collidable = true;
    collider_type = ColliderType::AABB_BOX;
    local_aabb.min = Vector3(-0.5f, 0.0f, -0.5f);
    local_aabb.max = Vector3(0.5f, 2.25f, 0.5f);

    Material material;
    visual = new EntityMesh(mesh, material);
    visual->is_collidable = false; // Importante: la malla gráfica no debe colisionar
    addChild(visual);
}

void Player::update(float delta_time)
{
    if (current_hp > 0.0f && model.getTranslation().y <= DEADZONE_Y) {
        takeDamage(current_hp);
    }
    if (flash_timer > 0.0f)
        flash_timer -= delta_time;

    if (ghost_delay > 0.0f) {
        ghost_delay -= delta_time;
    }
    else if (ghost_hp > current_hp) {
        ghost_hp += (current_hp - ghost_hp) * 6.0f * delta_time;
        if (ghost_hp - current_hp < 0.1f)
            ghost_hp = current_hp;
    }

    if (is_grounded && dash_time <= 0.0f) {
        can_dash = true;
    }

    if (dash_cooldown > 0.0f)
        dash_cooldown -= delta_time;

    if (dash_time > 0.0f) {
        dash_time -= delta_time;

        float dash_speed = DASH_METERS / DASH_DURATION;
        Vector3 movement = dash_dir * dash_speed * delta_time;

        // Colisión eje X
        Vector3 pos = model.getTranslation();
        model.setTranslation(pos + Vector3(movement.x, 0.0f, 0.0f));
        for (Entity* map_entity : World::getInstance()->root->children) {
            if (map_entity != this && map_entity->is_collidable && checkCollision(map_entity)) {
                model.setTranslation(pos);
                break;
            }
        }

        // Colisión eje Z
        pos = model.getTranslation();
        model.setTranslation(pos + Vector3(0.0f, 0.0f, movement.z));
        for (Entity* map_entity : World::getInstance()->root->children) {
            if (map_entity != this && map_entity->is_collidable && checkCollision(map_entity)) {
                model.setTranslation(pos);
                break;
            }
        }

        // Mantener la rotación del modelo
        Vector3 final_pos = model.getTranslation();
        model.setTranslation(final_pos.x, final_pos.y, final_pos.z);
        model.rotate(current_yaw, Vector3(0.0f, 1.0f, 0.0f));

        return; // Sale del update para no aplicar gravedad mientras dura el dash
    }

    if (punch_time > 0.0f) {
        punch_time -= delta_time;

        if (punch_time <= 0.0f) {
            punch_time = 0.0f;
            punch_cooldown = PUNCH_RECOVERY;
        }
    }
    else if (punch_cooldown > 0.0f) {
        punch_cooldown -= delta_time;
    }
    applyGravity(delta_time);
    Entity::update(delta_time);
}

void Player::move(Vector3 direction, float delta_time) {
    if (dash_time > 0.0f) return;
    float length = sqrt(direction.x * direction.x + direction.z * direction.z);

    if (length > 0.0f) {
        direction.x /= length;
        direction.z /= length;

        // 1. Calculamos hacia qué ángulo debe mirar según el movimiento
        if (punch_time <= 0.0f) {
            target_yaw = -atan2f(direction.x, direction.z);
        }
    }
    else {
        direction.x = 0.0f;
        direction.z = 0.0f;
    }

    Vector3 movement = direction * velocity * delta_time;

    // eje X
    Vector3 position = model.getTranslation();
    model.setTranslation(position + Vector3(movement.x, 0.0f, 0.0f));
    bool collision_x = false;

    for (Entity* map_entity : World::getInstance()->root->children) {
        if (map_entity == this || !map_entity->is_collidable)
            continue;

        if (checkCollision(map_entity)) {
            collision_x = true;
            break;
        }
    }

    if (collision_x)
        model.setTranslation(position);

    // eje Z
    position = model.getTranslation();
    model.setTranslation(position + Vector3(0.0f, 0.0f, movement.z));
    bool collision_z = false;

    for (Entity* map_entity : World::getInstance()->root->children) {
        if (map_entity == this || !map_entity->is_collidable)
            continue;

        if (checkCollision(map_entity)) {
            collision_z = true;
            break;
        }
    }

    if (collision_z)
        model.setTranslation(position);

    // 2. Interpolación suave del giro (estilo Fall Guys)
    if (punch_time <= 0.0f) {
        float diff = target_yaw - current_yaw;
        while (diff > 3.14159265f)  diff -= 6.28318530f;
        while (diff < -3.14159265f) diff += 6.28318530f;

        float turn_speed = 50.0f;
        float t = turn_speed * delta_time;
        if (t > 1.0f) t = 1.0f;
        current_yaw += diff * t;
    }

    // 3. Reconstruimos la matriz final con su posición válida y su rotación
    Vector3 final_pos = model.getTranslation();
    model.setTranslation(final_pos.x, final_pos.y, final_pos.z);
    model.rotate(current_yaw, Vector3(0.0f, 1.0f, 0.0f));
}

void Player::jump(float delta_time) {

    if (is_grounded) {
        vertical_velocity = 11.0f;
        is_grounded = false;

        is_jump_held = true;
        jump_hold_time = 0.0f;

        dash_time = 0.0f;
    }

}

void Player::stopJump() {
    is_jump_held = false;
}

void Player::dash() {
    if (!can_dash || dash_time > 0.0f || dash_cooldown > 0.0f)
        return;

    can_dash = false;
    dash_time = DASH_DURATION;
    dash_cooldown = 0.5f;
    dash_dir = Vector3(-sinf(current_yaw), 0.0f, cosf(current_yaw));
    vertical_velocity = 0.0f;
    is_grounded = false;
}

void Player::applyGravity(float delta_time)
{
    if (is_jump_held && vertical_velocity > 0.0f && jump_hold_time < MAX_JUMP_HOLD) {
        vertical_velocity += EXTRA_JUMP_FORCE * delta_time;
        jump_hold_time += delta_time;
    }

    vertical_velocity -= gravity * delta_time;
    float movement_y = vertical_velocity * delta_time;

    Vector3 position = model.getTranslation();
    model.setTranslation(position + Vector3(0.0f, movement_y, 0.0f));
    bool collision_y = false;

    for (Entity* map_entity : World::getInstance()->root->children){
        if (map_entity == this || !map_entity->is_collidable)
            continue;

        if (checkCollision(map_entity)){
            collision_y = true;
            break;
        }
    }

    if (collision_y) {
        model.setTranslation(position);

        if (movement_y < 0.0f)
            is_grounded = true;

        vertical_velocity = 0.0f;
    }
    else{
        is_grounded = false;
    }
    Vector3 final_pos = model.getTranslation();
    model.setTranslation(final_pos.x, final_pos.y, final_pos.z);
    model.rotate(current_yaw, Vector3(0.0f, 1.0f, 0.0f));
}

//This render function will be eliminated when animations are implemented.
//This is only to see the hitbox visually for debugging
void Player::render(Camera* camera)
{
    // 1. Render normal de la entidad (dibuja al personaje y sus hijos)
    Entity::render(camera);

    // 2. Dibujar directamente la AABB
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
        {0,1}, {1,2}, {2,3}, {3,0}, // Base inferior
        {4,5}, {5,6}, {6,7}, {7,4}, // Base superior
        {0,4}, {1,5}, {2,6}, {3,7}  // Bordes verticales
    };

    // Render de líneas directas en OpenGL
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 0.0f, 0.0f); // Rojo

    glBegin(GL_LINES);
    for (int i = 0; i < 12; ++i) {
        glVertex3fv(&corners[indices[i][0]].x);
        glVertex3fv(&corners[indices[i][1]].x);
    }
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f); // Restablecer color base

    if (punch_time > 0.0f) {
        PunchOBB obb = getPunchOBB();
        Vector3 rx = obb.right * obb.half_extents.x;
        Vector3 ry = Vector3(0.0f, obb.half_extents.y, 0.0f);
        Vector3 rz = obb.forward * obb.half_extents.z;

        Vector3 p_corners[8] = {
            obb.center - rx - ry - rz,
            obb.center + rx - ry - rz,
            obb.center + rx - ry + rz,
            obb.center - rx - ry + rz,
            obb.center - rx + ry - rz,
            obb.center + rx + ry - rz,
            obb.center + rx + ry + rz,
            obb.center - rx + ry + rz
        };

        glColor3f(1.0f, 0.85f, 0.0f); // Amarillo
        glBegin(GL_LINES);
        for (int i = 0; i < 12; ++i) {
            glVertex3fv(&p_corners[indices[i][0]].x);
            glVertex3fv(&p_corners[indices[i][1]].x);
        }
        glEnd();
    }

    glColor3f(1.0f, 1.0f, 1.0f);
}

bool Player::checkCollision(Entity* map_entity)
{
    if (!map_entity || !map_entity->is_collidable) return false;

    if (map_entity->collider_type == ColliderType::AABB_BOX) {
        return testAABBvsAABB(getAABB(), map_entity->getAABB());
    }

    return false;
}

void Player::takeDamage(float amount)
{
    if (current_hp <= 0.0f) return;

    current_hp -= amount;
    if (current_hp < 0.0f) current_hp = 0.0f;

    ghost_delay = 0.45f; // Espera 0.45s antes de bajar la barra secundaria
    flash_timer = 0.12f; // Destello y vibración al recibir el golpe
}

void Player::punch()
{
    if (punch_time > 0.0f || punch_cooldown > 0.0f || dash_time > 0.0f)
        return;

    punch_time = PUNCH_DURATION;
    current_yaw = target_yaw;
    PunchOBB punch_obb = getPunchOBB();
    for (Entity* map_entity : World::getInstance()->root->children) {
        if (map_entity == this) continue;

        if (testOBBvsAABB(punch_obb, map_entity->getAABB())) {
            Player* target_player = dynamic_cast<Player*>(map_entity);
            if (target_player) {
                target_player->takeDamage(PUNCH_DAMAGE);
            }
        }
    }
}

PunchOBB Player::getPunchOBB()
{
    PunchOBB obb;
    obb.forward = Vector3(-sinf(current_yaw), 0.0f, cosf(current_yaw));
    obb.right = Vector3(cosf(current_yaw), 0.0f, sinf(current_yaw));

    obb.half_extents = Vector3(0.4f, 0.4f, 0.6f);

    Vector3 pos = model.getTranslation();
    obb.center = pos + obb.forward * 0.9f + Vector3(0.0f, 1.1f, 0.0f);

    return obb;
}

bool Player::testOBBvsAABB(const PunchOBB& obb, const AABB& aabb)
{
    float obb_min_y = obb.center.y - obb.half_extents.y;
    float obb_max_y = obb.center.y + obb.half_extents.y;
    if (obb_max_y < aabb.min.y || obb_min_y > aabb.max.y)
        return false;

    float aabb_cx = (aabb.min.x + aabb.max.x) * 0.5f;
    float aabb_cz = (aabb.min.z + aabb.max.z) * 0.5f;
    float aabb_hx = (aabb.max.x - aabb.min.x) * 0.5f;
    float aabb_hz = (aabb.max.z - aabb.min.z) * 0.5f;

    float dx = obb.center.x - aabb_cx;
    float dz = obb.center.z - aabb_cz;

    float axes[4][2] = {
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { obb.right.x, obb.right.z },
        { obb.forward.x, obb.forward.z }
    };

    for (int i = 0; i < 4; ++i) {
        float ax = axes[i][0];
        float az = axes[i][1];

        float dist = fabsf(dx * ax + dz * az);

        float r_aabb = aabb_hx * fabsf(ax) + aabb_hz * fabsf(az);

        float r_obb = obb.half_extents.x * fabsf(obb.right.x * ax + obb.right.z * az)
            + obb.half_extents.z * fabsf(obb.forward.x * ax + obb.forward.z * az);

        if (dist > r_aabb + r_obb)
            return false;
    }

    return true;
}