#include "entityMesh.h"
#include "graphics/shader.h"
#include "graphics/texture.h"

EntityMesh::EntityMesh(Mesh* mesh, const Material& material)
    : mesh(mesh), material(material) {
    if (!this->material.shader) {
        this->material.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    }
    if (!this->material.diffuse) {
        this->material.diffuse = Texture::Get("data/textures/missing.tga");
    }
    if (mesh) {
        local_aabb.min = mesh->aabb_min;
        local_aabb.max = mesh->aabb_max;
    }
}

//Render the mesh in the world
void EntityMesh::render(Camera* camera) {
    if (!mesh) return;

    BoundingBox box = transformBoundingBox(
        getGlobalMatrix(),
        mesh->box
    );

    float distance = 50.0f;
    Vector3 center = box.center;

    // Discard objects far away from the camera
    if (camera->eye.distance(center) > distance)
        return;


    float sphere_radius = box.halfsize.length();

    // Discard objects whose bounding sphere 
    // is not inside the camera frustum
    if (camera->testSphereInFrustum(center,
        sphere_radius) != CLIP_INSIDE)
        return;


    // Testing box

    // Discard objects whose bounding box is completely outside the camera frustum
    if (camera->testBoxInFrustum(center,
        box.halfsize) == CLIP_OUTSIDE)
        return;


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
            //Preguntar porque esto es con parents
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