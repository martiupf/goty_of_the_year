#pragma once

#include "framework/entities/entity.h"
#include "game/entityMesh.h"

#include "graphics/material.h"

#include <map>

struct sRenderData {
    std::vector<Matrix44> models;
    Material material;
};

class SceneParser {

	std::map<std::string, sRenderData> meshes_to_load;

public:
	bool parse(const char* filename, Entity* root);
};