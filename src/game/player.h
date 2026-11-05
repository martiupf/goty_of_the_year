#pragma once

#include "framework/includes.h"
#include "framework/camera.h"
#include "framework/utils.h"
#include "graphics/mesh.h"

class Player
{
public:
	Vector3 position;
	Vector3 velocity;
	float rotation;

	Mesh* mesh;

	Player(Vector3 start_position);

	void update(double seconds_elapsed);
	void render();
};