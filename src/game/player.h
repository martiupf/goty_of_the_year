#pragma once


#include "framework/entities/entity.h"
#include "game/entityMesh.h"

class Player : public Entity
{
public:

	EntityMesh* visual;

	float velocity;
	float rotation;

	AABB collisionBox;

	Mesh* mesh;

	Player(Vector3 start_position, Mesh* mesh);
	void move(Vector3 direction, float delta_time);
	void update(double delta_time);
	void render(Camera* camera);
	//Returns AABB translated to the world
	bool checkCollision(Entity* map_entity);
};