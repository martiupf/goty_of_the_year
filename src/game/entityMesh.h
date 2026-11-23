#pragma once

#include "framework/entities/entity.h"
#include "graphics/mesh.h"
#include "world.h"

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
    bool receive_lighting = true;
    bool cast_shadows = true;
    bool receive_blob_shadows = true;
    std::vector<Matrix44> models;

    void render(Camera* camera) override;
    void update(float elapsed_time) override;
};