#pragma once

#include "framework/includes.h"
#include "framework/framework.h"

class Camera;

enum class ColliderType {
	AABB_BOX,
	SPHERE,
	OBB_BOX,
	MESH
};

struct AABB {
	Vector3 min;
	Vector3 max;
};

class Entity {

public:

	Entity() {}; 			// Constructor
	virtual ~Entity() {
		for (Entity* child : children)
			delete child;

		children.clear();
	}; 	// Destructor

	std::string name;
	ColliderType collider_type;
	AABB local_aabb;
	bool is_collidable = false;
	Matrix44 model;

	Entity* parent = nullptr;
	std::vector<Entity*> children;

	void addChild(Entity* child);
	void removeChild(Entity* child);

	// Methods that should be overwritten
	// by derived classes 
	virtual void render(Camera* camera);
	virtual void update(float delta_time);

	// Some useful methods
	Matrix44 getGlobalMatrix();
	float distance(Entity* e);
	virtual AABB getAABB();
};
