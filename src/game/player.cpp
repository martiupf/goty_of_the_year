#include "player.h"
#include <cmath>

Player::Player(Vector3 start_position, Mesh* mesh)
{
    
    model.setTranslation(start_position);
    
    velocity = 5.0f;
    rotation = 0.0f;

    Material material;
    visual = new EntityMesh(mesh, material);
    addChild(visual);
}

void Player::move(Vector3 direction, float delta_time) {
    float length = sqrt(direction.x * direction.x + direction.z * direction.z);

    if (length > 0.0f) {
        direction.x /= length;
        direction.z /= length;

        Vector3 current_position = model.getTranslation();

        current_position = current_position + direction * delta_time * velocity;
        model.setTranslation(current_position);
    }
}

void Player::update(double delta_time)
{
    Entity::update(delta_time);
}