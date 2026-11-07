#pragma once

#include "framework/entities/entity.h"
#include "graphics/mesh.h"

// Keep Material's helper struct distinct from SceneParser's global sRenderData.
#define sRenderData MaterialRenderData
#include "graphics/material.h"
#undef sRenderData

#include "framework/camera.h"

class EntityMesh : public Entity {
public:
    EntityMesh(Mesh* mesh, const Material& material);

    Mesh* mesh = nullptr;
    Material material;
    bool isInstanced = false;
    std::vector<Matrix44> models;

    void render(Camera* camera) override;
    void update(float elapsed_time) override;
};