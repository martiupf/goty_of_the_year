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
	mouse_locked = true;
	SDL_SetRelativeMouseMode(SDL_TRUE);

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

	loadMap("data/testing_map.scene");
	World* world = World::getInstance();
	Mesh* player_mesh = Mesh::Get("data/cuidado/Untitled.obj"); // se podrian poner dos meshes diferentes

	player1 = new Player(Vector3(-3, 2, 0), player_mesh);
	player2 = new Player(Vector3(3, 0.0, 0), player_mesh);

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

	renderHUD();

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
		if (Input::isMousePressed(SDL_BUTTON_LEFT) || mouse_locked)
		{
			float sensitivity = 0.05f;
			cam1_yaw -= Input::mouse_delta.x * 0.01f * sensitivity;
			cam1_pitch -= Input::mouse_delta.y * 0.01f * sensitivity;

			if (cam1_pitch < 0.05f) cam1_pitch = 0.05f;
			if (cam1_pitch > 1.4f)  cam1_pitch = 1.4f;
		}
		// PLAYER 1 (movimiento relativo a cam1_yaw)
		float input_z = 0.0f;
		float input_x = 0.0f;

		if (Input::isKeyPressed(SDL_SCANCODE_W)) input_z += 1.0f;
		if (Input::isKeyPressed(SDL_SCANCODE_S)) input_z -= 1.0f;
		if (Input::isKeyPressed(SDL_SCANCODE_A)) input_x += 1.0f;
		if (Input::isKeyPressed(SDL_SCANCODE_D)) input_x -= 1.0f;

		// Ejes horizontales de la cámara en el plano XZ
		Vector3 forward(-sinf(cam1_yaw), 0.0f, cosf(cam1_yaw));
		Vector3 left(cosf(cam1_yaw), 0.0f, sinf(cam1_yaw));

		// Dirección final en coordenadas del mundo (se normaliza dentro de move)
		Vector3 direction1 = (forward * input_z) + (left * input_x);

		player1->move(direction1, seconds_elapsed);

		if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) player1->jump(seconds_elapsed);
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) player1->dash();

		// PLAYER 2 (movimiento relativo a cam2_yaw)
		float input2_z = 0.0f;
		float input2_x = 0.0f;

		if (Input::gamepads[0].connected)
		{
			float deadzone = 0.2f;
			float sensitivity = 2.5f; // radianes por segundo

			float rx = Input::gamepads[0].axis[RIGHT_ANALOG_X];
			float ry = Input::gamepads[0].axis[RIGHT_ANALOG_Y];

			if (fabs(rx) > deadzone) cam2_yaw += rx * sensitivity * (float)seconds_elapsed;
			if (fabs(ry) > deadzone) cam2_pitch += ry * sensitivity * (float)seconds_elapsed;

			if (cam2_pitch < 0.05f) cam2_pitch = 0.05f;
			if (cam2_pitch > 1.4f)  cam2_pitch = 1.4f;

			float axis_x = Input::gamepads[0].axis[LEFT_ANALOG_X];
			float axis_y = Input::gamepads[0].axis[LEFT_ANALOG_Y];

			if (fabs(axis_y) > deadzone) input2_z = -axis_y;
			if (fabs(axis_x) > deadzone) input2_x = -axis_x;

			if (Input::gamepads[0].button[A_BUTTON]) player2->jump(seconds_elapsed);
			if (Input::gamepads[0].button[B_BUTTON]) player2->dash();
		}

		Vector3 forward2(-sinf(cam2_yaw), 0.0f, cosf(cam2_yaw));
		Vector3 left2(cosf(cam2_yaw), 0.0f, sinf(cam2_yaw));

		Vector3 direction2 = (forward2 * input2_z) + (left2 * input2_x);
		player2->move(direction2, seconds_elapsed);

		updateOrbitCamera();
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
	Vector3 target = player->model.getTranslation() + Vector3(0.0f,2.0f,0.0f);
	Vector3 offset(0.0f, 6.0f, -10.0f);
	Vector3 camera_position = target + offset;
	
	camera->lookAt(camera_position, target, Vector3(0.0f, 1.0f, 0.0f));
}

void Game::updateOrbitCamera() {
	Vector3 target1 = player1->model.getTranslation() + Vector3(0.0f, 2.0f, 0.0f);
	Vector3 target2 = player2->model.getTranslation() + Vector3(0.0f, 2.0f, 0.0f);

	float distance = 11.0f;
	Vector3 offset1;
	offset1.x = distance * cosf(cam1_pitch) * sinf(cam1_yaw);
	offset1.y = distance * sinf(cam1_pitch);
	offset1.z = -distance * cosf(cam1_pitch) * cosf(cam1_yaw);
	Vector3 offset2;
	offset2.x = distance * cosf(cam2_pitch) * sinf(cam2_yaw);
	offset2.y = distance * sinf(cam2_pitch);
	offset2.z = -distance * cosf(cam2_pitch) * cosf(cam2_yaw);

	Vector3 camera_position = target1 + offset1;
	camera1->lookAt(camera_position, target1, Vector3(0.0f, 1.0f, 0.0f));
	camera_position = target2 + offset2;
	camera2->lookAt(camera_position, target2, Vector3(0.0f, 1.0f, 0.0f));
}

void Game::drawClippedBar(float x, float y, float full_w, float h, float clip_w,
	float r_left, float g_left, float b_left,
	float r_right, float g_right, float b_right,
	float alpha)
{
	if (clip_w <= 0.0f || full_w <= 0.0f || h <= 0.0f) return;
	if (clip_w > full_w) clip_w = full_w;

	// Color interpolado en el punto exacto donde se corta la vida
	float t = clip_w / full_w;
	float r_end = r_left + (r_right - r_left) * t;
	float g_end = g_left + (g_right - g_left) * t;
	float b_end = b_left + (b_right - b_left) * t;

	glBegin(GL_QUADS);
	// Lado izquierdo
	glColor4f(r_left, g_left, b_left, alpha);
	glVertex2f(x, y);
	glVertex2f(x, y + h);

	// Lado derecho (en el corte de la vida actual)
	glColor4f(r_end, g_end, b_end, alpha);
	glVertex2f(x + clip_w, y + h);
	glVertex2f(x + clip_w, y);
	glEnd();
}

void Game::renderHealthBar(Player* player, float x, float y, float width, float height, bool invert_gradient)
{
	if (!player) return;

	if (player->flash_timer > 0.0f) {
		x += sinf(player->flash_timer * 90.0f) * 2.0f;
	}

	float hp_ratio = std::max(0.0f, std::min(1.0f, player->current_hp / player->max_hp));
	float ghost_ratio = std::max(0.0f, std::min(1.0f, player->ghost_hp / player->max_hp));

	// Capa 1: Sombra paralela
	drawClippedBar(x, y + (height * 0.12f), width, height, width,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.28f);

	// Capa 2: Fondo vacío rosado
	float bg_rl = 1.0f, bg_gl = 0.82f, bg_bl = 0.82f;
	float bg_rr = 1.0f, bg_gr = 0.72f, bg_br = 0.72f;
	if (invert_gradient) {
		std::swap(bg_rl, bg_rr);
		std::swap(bg_gl, bg_gr);
		std::swap(bg_bl, bg_br);
	}
	drawClippedBar(x, y, width, height, width,
		bg_rl, bg_gl, bg_bl,
		bg_rr, bg_gr, bg_br,
		0.92f);

	// Capa 3: Barra fantasma de daño retardado
	if (ghost_ratio > hp_ratio) {
		float gh_rl = 1.0f, gh_gl = 0.55f, gh_bl = 0.55f;
		float gh_rr = 1.0f, gh_gr = 0.65f, gh_br = 0.65f;
		if (invert_gradient) {
			std::swap(gh_rl, gh_rr);
			std::swap(gh_gl, gh_gr);
			std::swap(gh_bl, gh_br);
		}
		drawClippedBar(x, y, width, height, width * ghost_ratio,
			gh_rl, gh_gl, gh_bl,
			gh_rr, gh_gr, gh_br,
			0.75f);
	}

	// Capa 4: Barra de vida principal
	// Por defecto: rojo oscuro (izq) -> coral (der)
	// Si invert_gradient == true: coral (izq) -> rojo oscuro (der)
	float r_l = 0.82f, g_l = 0.02f, b_l = 0.02f;
	float r_r = 0.98f, g_r = 0.25f, b_r = 0.25f;

	if (player->flash_timer > 0.0f) {
		r_l = 1.0f; g_l = 0.65f; b_l = 0.65f;
		r_r = 1.0f; g_r = 0.85f; b_r = 0.85f;
	}

	if (invert_gradient) {
		std::swap(r_l, r_r);
		std::swap(g_l, g_r);
		std::swap(b_l, b_r);
	}

	drawClippedBar(x, y, width, height, width * hp_ratio,
		r_l, g_l, b_l,
		r_r, g_r, b_r,
		1.0f);
}

void Game::renderHUD()
{
	glViewport(0, 0, window_width, window_height);

	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, window_width, window_height, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float margin = 4.0f;
	float bar_h = window_height * health_bar_height_ratio;

	// Split-screen VERTICAL (Izquierda / Derecha)
	float viewport_w = window_width * 0.5f;
	float bar_w = viewport_w - (margin * 2.0f);

	renderHealthBar(player1, margin, margin, bar_w, bar_h, true);               // Invertido: parte oscura hacia el centro
	renderHealthBar(player2, viewport_w + margin, margin, bar_w, bar_h, false); // Normal: parte oscura hacia el centro

	// (Si cambias a split-screen HORIZONTAL Arriba/Abajo, usa estas dos líneas con false en ambas):
	/*
	float bar_w = window_width - (margin * 2.0f);
	renderHealthBar(player1, margin, margin, bar_w, bar_h, false);
	renderHealthBar(player2, margin, (window_height * 0.5f) + margin, bar_w, bar_h, false);
	*/

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}