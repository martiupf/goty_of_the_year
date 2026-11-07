#pragma once

#include "framework/entities/entity.h"
#include "graphics/mesh.h"
#include "graphics/material.h"
#include "framework/camera.h"

class EntityMesh : public Entity {
public:
    Mesh* mesh = nullptr;
    Material* material = nullptr;

    void render(Camera* camera) override;
    void update(float elapsed_time) override;
};