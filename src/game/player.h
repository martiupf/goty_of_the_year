#pragma once


#include "framework/entities/entity.h"
#include "game/entityMesh.h"

class Player : public Entity
{
public:

	EntityMesh* visual;

	AABB collisionBox;

	Mesh* mesh;

	float max_hp = 100.0f;
	float current_hp = 100.0f;
	float ghost_hp = 100.0f;
	float ghost_delay = 0.0f;
	float flash_timer = 0.0f;

	void takeDamage(float amount);

	float current_yaw = 0.0f;
	float target_yaw = 0.0f;
	float velocity;
	float rotation;

	float vertical_velocity;
	float gravity;
	bool is_grounded;

	float dash_time = 0.0f;
	float dash_cooldown = 0.0f; // Opcional, para no spamearlo en el aire
	Vector3 dash_dir;

	const float DASH_DURATION = 0.2f; // Segundos que dura el impulso
	const float DASH_METERS = 5.0f;   // Distancia total que recorre

	Player(Vector3 start_position, Mesh* mesh);
	void update(float delta_time) override;
	void move(Vector3 direction, float delta_time);
	void jump(float delta_time);
	void dash();
	void applyGravity(float delta_time);
	void render(Camera* camera);
	//Returns AABB translated to the world
	bool checkCollision(Entity* map_entity);
};