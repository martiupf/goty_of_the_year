#include "player.h"

Player::Player(Vector3 start_position)
{
    position = start_position;
    velocity = Vector3(0, 0, 0);
    rotation = 0.0f;

    mesh = NULL;
}

void Player::update(double seconds_elapsed)
{
}

void Player::render()
{
    if (!mesh)
        return;

    Matrix44 model;
}