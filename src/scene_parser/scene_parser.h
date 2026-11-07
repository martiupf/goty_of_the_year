#pragma once

#include "game/entityMesh.h"

#include "graphics/material.h"

#include <map>
#include <string>

struct sRenderData {
    std::vector<Matrix44> models;
    Material material;
    std::string texture_name;
};

class SceneParser {

    std::map<std::string, sRenderData> meshes_to_load;

public:
    bool parse(const char* filename, Entity* root);
};