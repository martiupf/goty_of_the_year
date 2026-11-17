#pragma once


#include "framework/entities/entity.h"
#include "game/entityMesh.h"

struct PunchOBB {
	Vector3 center;
	Vector3 half_extents;
	Vector3 right;
	Vector3 forward;
};

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

	float punch_time = 0.0f;
	float punch_cooldown = 0.0f;
	const float PUNCH_DURATION = 0.4f;
	const float PUNCH_RECOVERY = 0.5f;
	const float PUNCH_DAMAGE = 10.0f;
	const float DEADZONE_Y = -20.0f;

	void takeDamage(float amount);

	float current_yaw = 0.0f;
	float target_yaw = 0.0f;
	float velocity;
	float rotation;

	float vertical_velocity;
	float gravity;
	bool is_grounded;
	bool is_jump_held;
	float jump_hold_time;

	const float MAX_JUMP_HOLD = 0.25f;
	const float EXTRA_JUMP_FORCE = 15.0f;

	float dash_time = 0.0f;
	float dash_cooldown = 0.0f; // Opcional, para no spamearlo en el aire
	Vector3 dash_dir;

	bool can_dash = true;
	const float DASH_DURATION = 0.1f; // Segundos que dura el impulso
	const float DASH_METERS = 5.0f;   // Distancia total que recorre

	Player(Vector3 start_position, Mesh* mesh);
	void update(float delta_time) override;
	void move(Vector3 direction, float delta_time);
	void jump(float delta_time);
	void stopJump();
	void dash();
	void applyGravity(float delta_time);
	void render(Camera* camera);
	//Returns AABB translated to the world
	bool checkCollision(Entity* map_entity);

	void punch();
	PunchOBB getPunchOBB();
	bool testOBBvsAABB(const PunchOBB& obb, const AABB& aabb);
};