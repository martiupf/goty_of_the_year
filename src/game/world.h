#pragma once
#include "framework/entities/entity.h"

class World {
public:
    static World* instance;
    Entity* root = nullptr;

    World();
    ~World();

    static World* getInstance();

    void render(Camera* camera);
    void update(float elapsed_time);
};