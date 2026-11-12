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
        Vector3 original_min = mesh->aabb_min;
        Vector3 original_max = mesh->aabb_max;

        std::cout << "AABB ORIGINAL DEL MESH:" << std::endl;
        std::cout << "MIN: "
            << original_min.x << ", "
            << original_min.y << ", "
            << original_min.z << std::endl;

        std::cout << "MAX: "
            << original_max.x << ", "
            << original_max.y << ", "
            << original_max.z << std::endl;

        Vector3 corners[8] = {
            Vector3(original_min.x, original_min.y, original_min.z),
            Vector3(original_max.x, original_min.y, original_min.z),
            Vector3(original_min.x, original_max.y, original_min.z),
            Vector3(original_max.x, original_max.y, original_min.z),

            Vector3(original_min.x, original_min.y, original_max.z),
            Vector3(original_max.x, original_min.y, original_max.z),
            Vector3(original_min.x, original_max.y, original_max.z),
            Vector3(original_max.x, original_max.y, original_max.z)
        };

        local_aabb.min = Vector3(
            FLT_MAX, FLT_MAX, FLT_MAX
        );

        local_aabb.max = Vector3(
            -FLT_MAX, -FLT_MAX, -FLT_MAX
        );

        for (int i = 0; i < 8; i++) {

            // Conversión de ejes.
            Vector3 p(
                corners[i].x,
                -corners[i].z,
                corners[i].y
            );

            local_aabb.min.x = std::min(local_aabb.min.x, p.x);
            local_aabb.min.y = std::min(local_aabb.min.y, p.y);
            local_aabb.min.z = std::min(local_aabb.min.z, p.z);

            local_aabb.max.x = std::max(local_aabb.max.x, p.x);
            local_aabb.max.y = std::max(local_aabb.max.y, p.y);
            local_aabb.max.z = std::max(local_aabb.max.z, p.z);
        }

        std::cout << "AABB CONVERTIDA:" << std::endl;
        std::cout << "MIN: "
            << local_aabb.min.x << ", "
            << local_aabb.min.y << ", "
            << local_aabb.min.z << std::endl;

        std::cout << "MAX: "
            << local_aabb.max.x << ", "
            << local_aabb.max.y << ", "
            << local_aabb.max.z << std::endl;
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

    //debug
    shader->disable();

    // Dibujar los hijos normalmente
    Entity::render(camera);

    // Dibujar AABB si la entidad tiene colision
    if (is_collidable && collider_type == ColliderType::AABB_BOX)
    {
        AABB box = getAABB();

        Vector3 corners[8] = {
            Vector3(box.min.x, box.min.y, box.min.z),
            Vector3(box.max.x, box.min.y, box.min.z),
            Vector3(box.max.x, box.min.y, box.max.z),
            Vector3(box.min.x, box.min.y, box.max.z),
            Vector3(box.min.x, box.max.y, box.min.z),
            Vector3(box.max.x, box.max.y, box.min.z),
            Vector3(box.max.x, box.max.y, box.max.z),
            Vector3(box.min.x, box.max.y, box.max.z)
        };

        int indices[12][2] = {
            {0,1}, {1,2}, {2,3}, {3,0},
            {4,5}, {5,6}, {6,7}, {7,4},
            {0,4}, {1,5}, {2,6}, {3,7}
        };

        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        glColor3f(0.0f, 1.0f, 0.0f);

        glBegin(GL_LINES);

        for (int i = 0; i < 12; ++i)
        {
            glVertex3fv(&corners[indices[i][0]].x);
            glVertex3fv(&corners[indices[i][1]].x);
        }

        glEnd();

        glEnable(GL_DEPTH_TEST);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
}

void EntityMesh::update(float elapsed_time) {
    Entity::update(elapsed_time);
}