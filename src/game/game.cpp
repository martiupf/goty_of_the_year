#include "game.h"
#include "scene_parser/scene_parser.h"
#include <cmath>

#define LINE_WIDTH 4

float angle = 0;
float mouse_speed = 100.0f;

Game* Game::instance = NULL;

Game::Game(int window_width, int window_height, SDL_Window* window)
{
	printf("updated");
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	float half_aspect = ((window_width - LINE_WIDTH) * 0.5f) / (float)window_height;

	camera1 = new Camera();
	camera1->lookAt(Vector3(0.f, 15.f, 25.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera1->setPerspective(70.f, half_aspect, 0.1f, 10000.f);

	camera2 = new Camera();
	camera2->lookAt(Vector3(0.f, 15.f, 25.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera2->setPerspective(70.f, half_aspect, 0.1f, 10000.f);

	SDL_ShowCursor(!mouse_locked);


	camera_mode = FREE_CAMERA;

	loadMap("data/myscene.scene");
	World* world = World::getInstance();
	Mesh* player_mesh = Mesh::Get("data/cuidado/Untitled.obj"); // se podrian poner dos meshes diferentes

	player1 = new Player(Vector3(-3, 0, 0), player_mesh);
	player2 = new Player(Vector3(3, 0, 0), player_mesh);

	world->root->addChild(player1);
	world->root->addChild(player2);
}

bool Game::loadMap(const char* scene_filepath)
{
	//There's only one world instance. We get that instance
	World* world = World::getInstance();
	//If another map was loaded
	if (world->root && !world->root->children.empty()) {
		for (Entity* child : world->root->children) {
			delete child;
		}
		world->root->children.clear();
	}

	//Parse the map with SceneParser
	SceneParser parser;
	if (!parser.parse(scene_filepath, world->root)) {
		std::cerr << "[ERROR] Failed to load the map: " << scene_filepath << std::endl;
		return false;
	}

	std::cout << "[OK] Map successfully loaded: " << scene_filepath << std::endl;
	return true;
}

void Game::renderScene(Camera* camera)
{
	camera->enable();
	//This renders all meshes of the world
	World::getInstance()->render(camera);
	drawGrid();
}

void Game::render(void)
{
	int viewport_width = (window_width - LINE_WIDTH) / 2;

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, viewport_width, window_height);
	renderScene(camera1);

	glViewport(viewport_width + LINE_WIDTH, 0, viewport_width, window_height);
	renderScene(camera2);

	glViewport(0, 0, window_width, window_height);
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{
	float speed = (float)seconds_elapsed * mouse_speed;
	World::getInstance()->update((float)seconds_elapsed);

	angle += (float)seconds_elapsed * 10.0f;

	if (camera_mode == FREE_CAMERA) {
		if (Input::isMousePressed(SDL_BUTTON_LEFT) || mouse_locked)
		{
			camera1->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
			camera1->rotate(Input::mouse_delta.y * 0.005f, camera1->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
		}

		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 5;
		if (Input::isKeyPressed(SDL_SCANCODE_W)) camera1->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_S)) camera1->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_A)) camera1->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_D)) camera1->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);

		if (Input::isMousePressed(SDL_BUTTON_RIGHT))
		{
			camera2->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
			camera2->rotate(Input::mouse_delta.y * 0.005f, camera2->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
		}

		if (Input::isKeyPressed(SDL_SCANCODE_UP))    camera2->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_DOWN))  camera2->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_LEFT))  camera2->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera2->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	}

	else if (camera_mode == THIRD_PERSON) {
		
		// PLAYER 1
		Vector3 direction1(0, 0, 0);
		// PROVISIONAL: movimiento WASD
		if (Input::isKeyPressed(SDL_SCANCODE_W)) direction1.z -= 1;
		if (Input::isKeyPressed(SDL_SCANCODE_S)) direction1.z += 1;
		if (Input::isKeyPressed(SDL_SCANCODE_A)) direction1.x -= 1;
		if (Input::isKeyPressed(SDL_SCANCODE_D)) direction1.x += 1;

		player1->move(direction1, seconds_elapsed);

		// PLAYER 2
		Vector3 direction2(0, 0, 0);
		// PROVISIONAL: movimiento flechas
		if (Input::isKeyPressed(SDL_SCANCODE_UP)) direction2.z -= 1;
		if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) direction2.z += 1;
		if (Input::isKeyPressed(SDL_SCANCODE_LEFT)) direction2.x -= 1;
		if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) direction2.x += 1;

		player2->move(direction2, seconds_elapsed);


		updateThirdPersonCamera(camera1, player1);
		updateThirdPersonCamera(camera2, player2);
	}
}

void Game::onKeyDown(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_ESCAPE: must_exit = true; break;
	case SDLK_F1: Shader::ReloadAll(); break;
	case SDLK_F2:
		if (camera_mode == FREE_CAMERA) {
			camera_mode = THIRD_PERSON;
			updateThirdPersonCamera(camera1, player1);
			updateThirdPersonCamera(camera2, player2);
		}
		else { camera_mode = FREE_CAMERA; }
		break;
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event) {}

void Game::onMouseButtonDown(SDL_MouseButtonEvent event)
{
	if (event.button == SDL_BUTTON_MIDDLE)
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
		SDL_SetRelativeMouseMode((SDL_bool)(mouse_locked));
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event) {}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	mouse_speed *= event.y > 0 ? 1.1f : 0.9f;
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event) {}
void Game::onGamepadButtonUp(SDL_JoyButtonEvent event) {}

void Game::onResize(int width, int height)
{
	std::cout << "window resized: " << width << "," << height << std::endl;
	window_width = width;
	window_height = height;
	float half_aspect = ((window_width - LINE_WIDTH) * 0.5f) / (float)window_height;
	if (camera1) camera1->aspect = half_aspect;
	if (camera2) camera2->aspect = half_aspect;
}

void Game::setMouseLocked(bool must_lock)
{
	SDL_ShowCursor(!must_lock);
	SDL_SetRelativeMouseMode((SDL_bool)must_lock);
	mouse_locked = must_lock;
}

void Game::updateThirdPersonCamera(Camera* camera, Player* player) {
	Vector3 target = player->model.getTranslation();
	Vector3 offset(0.0f, 6.0f, 10.0f);
	Vector3 camera_position = target + offset;

	camera->lookAt(camera_position, target, Vector3(0.0f, 1.0f, 0.0f));
}