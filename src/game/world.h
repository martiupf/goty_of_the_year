#pragma once
#include "framework/entities/entity.h"
#include "../graphics/texture.h"

class World {
public:
    static World* instance;
    Entity* root = nullptr;

    Vector3 light_dir = Vector3(0.4f, -1.0f, 0.3f);
    Vector3 light_color = Vector3(1.0f, 0.95f, 0.85f);
    Vector3 ambient_light = Vector3(0.3f, 0.3f, 0.35f);

    Texture* shadow_map = nullptr;
    Matrix44 light_viewprojection;
    bool is_shadow_pass = false;

    Vector3 player1_pos;
    Vector3 player2_pos;
    bool has_player1 = false;
    bool has_player2 = false;

    Texture* player1_shadow_map = nullptr;
    Matrix44 player1_vp;
    Texture* player2_shadow_map = nullptr;
    Matrix44 player2_vp;

    World();
    ~World();

    static World* getInstance();

    void render(Camera* camera);
    void update(float elapsed_time);
};