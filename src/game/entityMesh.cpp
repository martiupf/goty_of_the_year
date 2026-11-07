#include "entityMesh.h"
#include "graphics/shader.h"

EntityMesh::EntityMesh(Mesh* mesh, const Material& material)
    : mesh(mesh), material(material) {
}

void EntityMesh::render(Camera* camera) {
    if (!mesh) return;

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    Shader* shader = material.shader;
    if (!shader) return;

    shader->enable();
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);

    if (material.diffuse) {
        shader->setTexture("u_texture", material.diffuse, 0);
    }
    shader->setUniform("u_color", material.color);

    if (isInstanced && !models.empty()) {
        Matrix44 parent_matrix;
        if (parent) {
            parent_matrix = parent->getGlobalMatrix();
        }

        for (const Matrix44& instance_model : models) {
            Matrix44 global_matrix = parent ? instance_model * parent_matrix : instance_model;
            shader->setUniform("u_model", global_matrix);
            mesh->render(GL_TRIANGLES);
        }
    }
    else {
        shader->setUniform("u_model", getGlobalMatrix());
        mesh->render(GL_TRIANGLES);
    }

    shader->disable();

    Entity::render(camera);
}

void EntityMesh::update(float elapsed_time) {
    Entity::update(elapsed_time);
}