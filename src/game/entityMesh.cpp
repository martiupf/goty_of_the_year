#include "entityMesh.h"
#include "graphics/shader.h"

void EntityMesh::render(Camera* camera) {
    if (!mesh || !material) return;

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    Shader* shader = material->shader;
    if (!shader) return;

    shader->enable();

    // 1. Usar la matriz global de la jerarquía
    Matrix44 global_matrix = getGlobalMatrix();
    shader->setUniform("u_model", global_matrix);
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);

    if (material->diffuse) {
        shader->setTexture("u_texture", material->diffuse, 0);
    }
    shader->setUniform("u_color", material->color);

    // Dibujar la malla
    mesh->render(GL_TRIANGLES);

    shader->disable();

    // 2. Propagar el render a entidades hijas de esta malla
    Entity::render(camera);
}

void EntityMesh::update(float elapsed_time) {
    Entity::update(elapsed_time);
}