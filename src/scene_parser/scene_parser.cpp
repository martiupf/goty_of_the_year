
#include "scene_parser.h"

#include "graphics/material.h"
#include "graphics/mesh.h"

#include "framework/utils.h"

#include <fstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <cstdlib>

bool SceneParser::parse(const char* filename, Entity* root)
{
    meshes_to_load.clear();

    std::cout << " + Scene loading: "
        << filename << "..." << std::endl;

    std::ifstream file(filename);

    if (!file.good()) {
        std::cerr << "Scene [ERROR] File not found!"
            << std::endl;
        return false;
    }

    std::string scene_info;
    std::string mesh_name;
    std::string model_data;

    file >> scene_info;
    file >> scene_info;

    int mesh_count = 0;

    // Leer los objetos de la escena.
    while (file >> mesh_name >> model_data)
    {
        if (mesh_name.empty() || mesh_name[0] == '#')
            continue;

        // Obtener los 16 valores de la matriz.
        std::vector<std::string> tokens =
            tokenize(model_data, ",");

        Matrix44 model;

        for (int t = 0; t < tokens.size() && t < 16; ++t) {
            model.m[t] = (float)atof(tokens[t].c_str());
        }

        // Agrupar objetos que utilizan la misma malla.
        sRenderData& render_data = meshes_to_load[mesh_name];

        render_data.models.push_back(model);

        mesh_count++;
    }

    // Crear las entidades.
    for (auto data : meshes_to_load)
    {
        mesh_name = "data/" + data.first;

        sRenderData& render_data = data.second;

        if (render_data.models.empty())
            continue;

        Material mat = render_data.material;

        EntityMesh* new_entity = nullptr;

        Mesh* mesh = Mesh::Get(mesh_name.c_str());

        if (!mesh) {
            std::cerr << "Error loading mesh: "
                << mesh_name << std::endl;
            continue;
        }

        new_entity = new EntityMesh(mesh, mat);

        if (!new_entity)
            continue;

        // Guardar el nombre original.
        new_entity->name = data.first;

        // Convertir el nombre a minúsculas.
        std::string name_lower = data.first;

        std::transform(
            name_lower.begin(),
            name_lower.end(),
            name_lower.begin(),
            [](unsigned char c) {
                return (char)std::tolower(c);
            }
        );

        // Detectar objetos colisionables.
        if (name_lower.find("aabb") != std::string::npos)
        {
            new_entity->is_collidable = true;
            new_entity->collider_type =
                ColliderType::AABB_BOX;

            std::cout << "[COLLIDER] "
                << data.first
                << " -> AABB"
                << std::endl;
        }
        else
        {
            new_entity->is_collidable = false;
            new_entity->collider_type =
                ColliderType::NO_COLLISION;
        }

        // Mostrar el resultado DESPUÉS de asignarlo.
        std::cout << "[SCENE ENTITY] "
            << "[" << new_entity->name << "]"
            << " | Collidable: "
            << new_entity->is_collidable
            << " | Collider type: "
            << (int)new_entity->collider_type
            << std::endl;

        // Configurar instancing.
        if (render_data.models.size() > 1)
        {
            new_entity->isInstanced = true;
            new_entity->models = render_data.models;
        }
        else
        {
            new_entity->model = render_data.models[0];
        }

        // Añadir la entidad al escenario.
        root->addChild(new_entity);
    }

    std::cout << "Scene [OK] Meshes added: "
        << mesh_count << std::endl;

    return true;
}
