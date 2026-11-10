#include "player.h"
#include "collider.h"
#include "world.h"
#include <cmath>

Player::Player(Vector3 start_position, Mesh* mesh)
{
    
    model.setTranslation(start_position);
    
    velocity = 5.0f;
    rotation = 0.0f;

    is_collidable = true;
    collider_type = ColliderType::AABB_BOX;
    local_aabb.min = Vector3(-0.5f, 0.0f, -0.5f);
    local_aabb.max = Vector3(0.5f, 2.25f, 0.5f);

    Material material;
    visual = new EntityMesh(mesh, material);
    visual->is_collidable = false; // Importante: la malla gráfica no debe colisionar
    addChild(visual);
}

void Player::move(Vector3 direction, float delta_time) {
    float length = sqrt(direction.x * direction.x + direction.z * direction.z);

    if (length <= 0.0f)
        return;

    direction.x /= length;
    direction.z /= length;

    Vector3 current_position = model.getTranslation();
    // debug
    if (!std::isfinite(current_position.x) ||
        !std::isfinite(current_position.y) ||
        !std::isfinite(current_position.z)) {

        std::cout << "ERROR: posicion actual invalida" << std::endl;
        return;
    }
    //

    Vector3 new_position = current_position + direction * delta_time * velocity;
    // debug
    if (!std::isfinite(new_position.x) ||
        !std::isfinite(new_position.y) ||
        !std::isfinite(new_position.z)) {

            std::cout << "ERROR: nueva posicion invalida" << std::endl;
            return;
    }
    //
    model.setTranslation(new_position);
    //debug
    std::cout << "Posicion nueva: "
        << new_position.x << ", "
        << new_position.y << ", "
        << new_position.z << std::endl;
    //
    // Recorre la lista de map_entities del world para ver si colisiona con alguna 
    
    bool has_collided = false;
    for (Entity* map_entity : World::getInstance()->root->children) {
        if (checkCollision(map_entity)) {
            //debug
            std::cout << "COLISION con entidad: " << map_entity->name 
                << " min.x: " << map_entity->getAABB().min.x 
                << " min.y: " << map_entity->getAABB().min.y
                << " min.z: " << map_entity->getAABB().min.z
                << " max.x: " << map_entity->getAABB().max.x
                << " max.y: " << map_entity->getAABB().max.y
                << " max.z: " << map_entity->getAABB().max.z
                << std::endl;
            //
            has_collided = true;
            break;
        }
    }

    if (has_collided)
        model.setTranslation(current_position);
}

void Player::update(double delta_time)
{
    Entity::update(delta_time);
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