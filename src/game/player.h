#pragma once


#include "framework/entities/entity.h"
#include "game/entityMesh.h"


//Para Pedro: necesitamos que esto herede de entity
class Player : public Entity
{
public:

	EntityMesh* visual;

	float velocity;
	float rotation;

	Mesh* mesh;

	Player(Vector3 start_position, Mesh* mesh);
	void move(Vector3 direction, float delta_time);
	void update(double delta_time);
};