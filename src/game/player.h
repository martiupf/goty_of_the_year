#pragma once


#include "framework/entities/entity.h"
#include "game/entityMesh.h"

class Player : public Entity
{
public:

	EntityMesh* visual;

	AABB collisionBox;

	Mesh* mesh;

	float velocity;
	float rotation;

	float vertical_velocity;
	float gravity;
	bool is_grounded;

	Player(Vector3 start_position, Mesh* mesh);
	void update(float delta_time) override;
	void move(Vector3 direction, float delta_time);
	void jump(float delta_time);
	void applyGravity(float delta_time);
	void render(Camera* camera);
	//Returns AABB translated to the world
	bool checkCollision(Entity* map_entity);
};