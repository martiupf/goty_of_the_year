#include "world.h"

World* World::instance = nullptr;

World::World() {
    instance = this;
    root = new Entity();
    root->name = "root";
}

World::~World() {
    if (root) delete root;
}

World* World::getInstance() {
    if (!instance) {
        instance = new World();
    }
    return instance;
}

void World::render(Camera* camera) {
    //This renders all the meshes by recursively calling the entity->render method until the last mesh comes and it doesn't have a child
    if (root) root->render(camera);
}

void World::update(float elapsed_time) {
    if (root) root->update(elapsed_time);
}