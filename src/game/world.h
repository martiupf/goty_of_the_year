#pragma once
#include "framework/entities/entity.h"

class World {
public:
    static World* instance;
    Entity* root = nullptr;

    Vector3 light_dir = Vector3(0.4f, -1.0f, 0.3f);
    Vector3 light_color = Vector3(1.0f, 0.95f, 0.85f);
    Vector3 ambient_light = Vector3(0.3f, 0.3f, 0.35f);

    World();
    ~World();

    static World* getInstance();

    void render(Camera* camera);
    void update(float elapsed_time);
};