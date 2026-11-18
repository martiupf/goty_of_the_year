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

	const float MAX_JUMP_HOLD = 0.22f;
	const float JUMP_SPEED = 13.0f;
	const float JUMP_RELEASE_SPEED = 4.0f;
	const float MAX_FALL_SPEED = -24.0f;

	void takeDamage(float amount);

	float current_yaw = 0.0f;
	float target_yaw = 0.0f;
	float velocity;
	float rotation;

	float vertical_velocity;
	float gravity;
	bool is_grounded = false;
	bool is_jump_held = false;
	float jump_hold_time = 0.0f;
	bool jump_pressed = false;
	float knockback_stun_timer = 0.0f;

	const float EXTRA_JUMP_FORCE = 45.0f;

	Vector3 knockback_vel = Vector3(0.0f, 0.0f, 0.0f);

	void punch();
	void applyKnockback(Vector3 direction);
	std::vector<Entity*> punch_hit_entities;

	float dash_time = 0.0f;
	bool can_jump_after_dash = false;
	bool jump_queued = false;
	float post_dash_jump_timer = 0.0f;
	float time_since_grounded = 0.0f;
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

	PunchOBB getPunchOBB();
	bool testOBBvsAABB(const PunchOBB& obb, const AABB& aabb);
};