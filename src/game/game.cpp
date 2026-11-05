#include "game.h"
#include "framework/utils.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"
#include "graphics/shader.h"
#include "framework/input.h"

#include <cmath>

#define LINE_WIDTH 4

//some globals
Mesh* mesh = NULL;
Texture* texture = NULL;
Shader* shader = NULL;
float angle = 0;
float mouse_speed = 100.0f;

Game* Game::instance = NULL;

Game::Game(int window_width, int window_height, SDL_Window* window)
{
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

	// OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	float half_aspect = ((window_width - LINE_WIDTH) * 0.5f) / (float)window_height;

	// Cambiado (0, 100, 100) -> (0, 15, 25) para encuadrar la arena de 20x20m de cerca
	camera1 = new Camera();
	camera1->lookAt(Vector3(0.f, 15.f, 25.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera1->setPerspective(70.f, half_aspect, 0.1f, 10000.f);

	camera2 = new Camera();
	camera2->lookAt(Vector3(0.f, 15.f, 25.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera2->setPerspective(70.f, half_aspect, 0.1f, 10000.f);

	// Load one texture using the Texture Manager
	texture = Texture::Get("data/textures/texture.tga");
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");

	mesh = Mesh::Get("data/meshes/testing_map.obj");

	// Hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse

	//n
	player1 = new Player(Vector3(0, 0, 0));
	player2 = new Player(Vector3(10, 0, 0));

	camera_mode = FREE_CAMERA;
}

void Game::renderScene(Camera* camera)
{
	// Activar la cámara recibida por parámetro
	camera->enable();

	// Render flags
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// CAMBIADO: Quitar m.rotate() para mantener el mapa estático en el origen (0,0,0)
	Matrix44 m;

	if (shader && mesh)
	{
		shader->enable();
		shader->setUniform("u_color", Vector4(1, 0, 0, 1));
		shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
		shader->setUniform("u_model", m);

		// REMOVIDO: u_texture y u_time (flat.fs no requiere textura ni tiempo)

		mesh->render(GL_TRIANGLES);

		shader->disable();
	}

	drawGrid();
}

//what to do when the image has to be draw
void Game::render(void)
{
	int viewport_width = (window_width - LINE_WIDTH) / 2;

	// Limpiar toda la pantalla en negro y el Z-buffer
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// --------------------------------------------------
	// VIEWPORT JUGADOR 1 (Lado Izquierdo)
	// --------------------------------------------------
	glViewport(0, 0, viewport_width, window_height);
	renderScene(camera1);

	// --------------------------------------------------
	// VIEWPORT JUGADOR 2 (Lado Derecho)
	// Se desplaza `viewport_width + line_width` píxeles a la derecha
	// --------------------------------------------------
	glViewport(viewport_width + LINE_WIDTH, 0, viewport_width, window_height);
	renderScene(camera2);

	// --------------------------------------------------
	// UI OVERLAY (Pantalla Completa)
	// --------------------------------------------------
	glViewport(0, 0, window_width, window_height);
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{
	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	// Example
	angle += (float)seconds_elapsed * 10.0f;

	if (camera_mode == FREE_CAMERA) {
		// Mouse input to rotate the cam
		if (Input::isMousePressed(SDL_BUTTON_LEFT) || mouse_locked) //is left button pressed?
		{
			camera1->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
			camera1->rotate(Input::mouse_delta.y * 0.005f, camera1->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
		}

		// Async input to move the camera around
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
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
		updateThirdPersonCamera(camera1, player1);
		updateThirdPersonCamera(camera2, player2);
	}
}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
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

void Game::onKeyUp(SDL_KeyboardEvent event)
{

}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
		SDL_SetRelativeMouseMode((SDL_bool)(mouse_locked));
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{

}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	mouse_speed *= event.y > 0 ? 1.1f : 0.9f;
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

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

//n
void Game::updateThirdPersonCamera(Camera* camera, Player* player) {
	Vector3 target = player->position;
	// distancia de la cámara al jugador
	Vector3 offset(0.0f, 6.0f, 10.0f);
	Vector3 camera_position = target + offset;

	camera->lookAt(camera_position, target, Vector3(0.0f, 1.0f, 0.0f));
}