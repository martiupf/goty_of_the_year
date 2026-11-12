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
    gravity = 10.0f;
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
        target_yaw = -atan2f(direction.x, direction.z);
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
    float diff = target_yaw - current_yaw;
    while (diff > 3.14159265f)  diff -= 6.28318530f;
    while (diff < -3.14159265f) diff += 6.28318530f;
    
    float turn_speed = 50.0f;
    float t = turn_speed * delta_time;
    if (t > 1.0f) t = 1.0f;
    current_yaw += diff * t;

    // 3. Reconstruimos la matriz final con su posición válida y su rotación
    Vector3 final_pos = model.getTranslation();
    model.setTranslation(final_pos.x, final_pos.y, final_pos.z);
    model.rotate(current_yaw, Vector3(0.0f, 1.0f, 0.0f));
}

void Player::jump(float delta_time) {

    if (is_grounded) {
        vertical_velocity += 10;
    }

}

void Player::dash() {
    if (dash_time > 0.0f || dash_cooldown > 0.0f) return;

    dash_time = DASH_DURATION;
    dash_cooldown = 0.8f;
    dash_dir = Vector3(-sinf(current_yaw), 0.0f, cosf(current_yaw));
    vertical_velocity = 0.0f;
}

void Player::applyGravity(float delta_time)
{
    vertical_velocity -= gravity * (float)delta_time;
    float movement_y = vertical_velocity * (float)delta_time;

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
}

bool Player::checkCollision(Entity* map_entity)
{
    if (!map_entity || !map_entity->is_collidable) return false;

    if (map_entity->collider_type == ColliderType::AABB_BOX) {
        return testAABBvsAABB(getAABB(), map_entity->getAABB());
    }

    return false;
}