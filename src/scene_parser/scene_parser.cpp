// scene_parser.cpp
#include "scene_parser.h"

#include "graphics/material.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"

#include "framework/utils.h"

#include <fstream>
#include <sstream> // Necesario para std::stringstream

bool SceneParser::parse(const char* filename, Entity* root)
{
    std::cout << " + Scene loading: " << filename << "..." << std::endl;

    std::ifstream file(filename);

    if (!file.good()) {
        std::cerr << "Scene [ERROR]" << " File not found!" << std::endl;
        return false;
    }

    std::string line;
    int mesh_count = 0;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string mesh_name, model_data, texture_name;

        // Leemos como mínimo la malla y la matriz
        if (ss >> mesh_name >> model_data)
        {
            // Si la línea no tiene 3ª columna, asignamos la textura por defecto
            if (!(ss >> texture_name)) {
                texture_name = "missing.tga";
            }

            std::vector<std::string> tokens = tokenize(model_data, ",");
            Matrix44 model;
            for (int t = 0; t < tokens.size(); ++t) {
                model.m[t] = (float)atof(tokens[t].c_str());
            }

            sRenderData& render_data = meshes_to_load[mesh_name];
            render_data.models.push_back(model);
            render_data.texture_name = texture_name;
            mesh_count++;
        }
    }

    // Instanciar entidades...
    for (auto data : meshes_to_load) {
        std::string mesh_name = "data/" + data.first;
        sRenderData& render_data = data.second;

        if (render_data.models.empty())
            continue;

        Material mat;
        mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
        mat.diffuse = Texture::Get(("data/textures/" + render_data.texture_name).c_str());
        mat.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

        EntityMesh* new_entity = nullptr;
        size_t tag = data.first.find("@tag");

        if (tag != std::string::npos) {
            Mesh* mesh = Mesh::Get("...");
        }
        else {
            Mesh* mesh = Mesh::Get(mesh_name.c_str());
            new_entity = new EntityMesh(mesh, mat);
        }

        if (!new_entity) continue;

        new_entity->name = data.first;

        if (render_data.models.size() > 1) {
            new_entity->isInstanced = true;
            new_entity->models = render_data.models;
        }
        else {
            new_entity->model = render_data.models[0];
        }

        root->addChild(new_entity);
    }

    std::cout << "Scene [OK]" << " Meshes added: " << mesh_count << std::endl;
    return true;
}