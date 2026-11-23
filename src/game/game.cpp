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
	SDL_ShowCursor(SDL_ENABLE);
	SDL_SetRelativeMouseMode(SDL_FALSE);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	initTitle();
}

void Game::initTitle() {
	start_button = new Button(300, 200, 200, 80, Texture::Get("data/textures/play_button.png") );
}

void Game::initBattle() {

	float half_aspect = ((window_width - LINE_WIDTH) * 0.5f) / (float)window_height;

	camera1 = new Camera();
	camera1->lookAt(Vector3(0.f, 15.f, 25.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera1->setPerspective(70.f, half_aspect, 0.1f, 10000.f);

	camera2 = new Camera();
	camera2->lookAt(Vector3(0.f, 15.f, 25.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera2->setPerspective(70.f, half_aspect, 0.1f, 10000.f);

	if (!shadow_fbo) {
		shadow_fbo = new FBO();
		shadow_fbo->setDepthOnly(2048, 2048);
	}
	if (!player1_shadow_fbo) {
		player1_shadow_fbo = new FBO();
		player1_shadow_fbo->setDepthOnly(256, 256);
	}
	if (!player2_shadow_fbo) {
		player2_shadow_fbo = new FBO();
		player2_shadow_fbo->setDepthOnly(256, 256);
	}
	if (!light_camera) {
		light_camera = new Camera();
	}

	SDL_ShowCursor(!mouse_locked);

	camera_mode = THIRD_PERSON;

	loadMap("data/testing_map.scene");
	World* world = World::getInstance();
	Mesh* player_mesh = Mesh::Get("data/cuidado/Untitled.obj"); // se podrian poner dos meshes diferentes

	player1 = new Player(Vector3(-3, 0, 0), player_mesh);
	player2 = new Player(Vector3(3, 0, 0), player_mesh);

	world->root->addChild(player1);
	world->root->addChild(player2);

	Mesh* sword_mesh = Mesh::Get("data/cuidado/Untitled.obj");

	Weapon* sword = new Weapon(Weapon::MELEE, sword_mesh);
	sword->setOnGround(Vector3(0.0f, 0.0f, 0.0f));

	world->root->addChild(sword);
}

void Game::initVictoryScreen() {
	replay_button = new Button(300, 200, 200, 80, Texture::Get("data/textures/play_button.png"));
	return_title_button = new Button(500, 200, 200, 80, Texture::Get("data/textures/play_button.png"));
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

	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	if (!skybox_tex) {
		skybox_tex = new Texture();
	}

	std::vector<std::string> faces = {
		"data/skybox/right.png",  // +X
		"data/skybox/left.png",   // -X
		"data/skybox/top.png",    // +Y
		"data/skybox/bottom.png", // -Y
		"data/skybox/front.png",  // +Z
		"data/skybox/back.png"    // -Z
	};
	skybox_tex->loadCubemap("skybox", faces);

	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex->texture_id);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	skybox_mesh = Mesh::Get("data/scene/aabb_Cubo.001/aabb_Cubo.001.obj");
	skybox_shader = Shader::Get("data/shaders/skybox.vs", "data/shaders/skybox.fs");

	// Calcular la caja envolvente (AABB) de todas las entidades del mapa que proyectan sombra
	shadow_box_min = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	shadow_box_max = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	bool has_casters = false;

	for (Entity* child : world->root->children) {
		EntityMesh* em = dynamic_cast<EntityMesh*>(child);
		if (!em || !em->mesh || !em->cast_shadows) continue;

		if (em->isInstanced && !em->models.empty()) {
			for (const Matrix44& m : em->models) {
				BoundingBox b = transformBoundingBox(m, em->mesh->box);
				Vector3 b_min = b.center - b.halfsize;
				Vector3 b_max = b.center + b.halfsize;
				shadow_box_min.x = std::min(shadow_box_min.x, b_min.x);
				shadow_box_min.y = std::min(shadow_box_min.y, b_min.y);
				shadow_box_min.z = std::min(shadow_box_min.z, b_min.z);
				shadow_box_max.x = std::max(shadow_box_max.x, b_max.x);
				shadow_box_max.y = std::max(shadow_box_max.y, b_max.y);
				shadow_box_max.z = std::max(shadow_box_max.z, b_max.z);
				has_casters = true;
			}
		}
		else {
			BoundingBox b = transformBoundingBox(em->getGlobalMatrix(), em->mesh->box);
			Vector3 b_min = b.center - b.halfsize;
			Vector3 b_max = b.center + b.halfsize;
			shadow_box_min.x = std::min(shadow_box_min.x, b_min.x);
			shadow_box_min.y = std::min(shadow_box_min.y, b_min.y);
			shadow_box_min.z = std::min(shadow_box_min.z, b_min.z);
			shadow_box_max.x = std::max(shadow_box_max.x, b_max.x);
			shadow_box_max.y = std::max(shadow_box_max.y, b_max.y);
			shadow_box_max.z = std::max(shadow_box_max.z, b_max.z);
			has_casters = true;
		}
	}

	if (!has_casters) {
		shadow_box_min = Vector3(-35.0f, -10.0f, -35.0f);
		shadow_box_max = Vector3(35.0f, 35.0f, 35.0f);
	}

	std::cout << "[OK] Map successfully loaded: " << scene_filepath << std::endl;
	return true;

}

void Game::renderShadowMap(void)
{
	if (!shadow_fbo || !light_camera) return;

	World* world = World::getInstance();
	Vector3 dir = world->light_dir;
	dir.normalize();

	// Ajusta el rango (-35..35) según el tamaño de tu mapa
	Vector3 target = (shadow_box_min + shadow_box_max) * 0.5f;
	float radius = (shadow_box_max - shadow_box_min).length() * 0.5f;
	Vector3 eye = target - dir * (radius + 10.0f);
	Vector3 up = (fabsf(dir.y) > 0.99f) ? Vector3(0.0f, 0.0f, 1.0f) : Vector3(0.0f, 1.0f, 0.0f);

	light_camera->lookAt(eye, target, up);

	// Transformar las 8 esquinas del mapa al espacio vista de la luz
	Vector3 corners[8] = {
		Vector3(shadow_box_min.x, shadow_box_min.y, shadow_box_min.z),
		Vector3(shadow_box_max.x, shadow_box_min.y, shadow_box_min.z),
		Vector3(shadow_box_min.x, shadow_box_max.y, shadow_box_min.z),
		Vector3(shadow_box_max.x, shadow_box_max.y, shadow_box_min.z),
		Vector3(shadow_box_min.x, shadow_box_min.y, shadow_box_max.z),
		Vector3(shadow_box_max.x, shadow_box_min.y, shadow_box_max.z),
		Vector3(shadow_box_min.x, shadow_box_max.y, shadow_box_max.z),
		Vector3(shadow_box_max.x, shadow_box_max.y, shadow_box_max.z)
	};

	float min_x = FLT_MAX, max_x = -FLT_MAX;
	float min_y = FLT_MAX, max_y = -FLT_MAX;
	float min_z = FLT_MAX, max_z = -FLT_MAX;

	for (int i = 0; i < 8; ++i) {
		Vector3 p = light_camera->view_matrix * corners[i];
		min_x = std::min(min_x, p.x);
		max_x = std::max(max_x, p.x);
		min_y = std::min(min_y, p.y);
		max_y = std::max(max_y, p.y);
		min_z = std::min(min_z, p.z);
		max_z = std::max(max_z, p.z);
	}

	float near_plane = std::max(0.1f, -max_z - 5.0f);
	float far_plane = -min_z + 5.0f;

	light_camera->setOrthographic(min_x, max_x, min_y, max_y, near_plane, far_plane);

	world->light_viewprojection = light_camera->viewprojection_matrix;
	world->shadow_map = shadow_fbo->depth_texture;
	world->is_shadow_pass = true;

	shadow_fbo->bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	world->render(light_camera);
	shadow_fbo->unbind();

	if (player1 && player1_shadow_fbo) {
		Vector3 p1 = world->player1_pos;
		light_camera->lookAt(p1 + Vector3(0.0f, 0.25f, 0.0f), p1 + Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f));
		light_camera->setOrthographic(-1.0f, 1.0f, -1.0f, 1.0f, 0.05f, 15.0f);
		world->player1_vp = light_camera->viewprojection_matrix;
		world->player1_shadow_map = player1_shadow_fbo->depth_texture;

		player1_shadow_fbo->bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		world->render(light_camera);
		player1_shadow_fbo->unbind();
	}

	if (player2 && player2_shadow_fbo) {
		Vector3 p2 = world->player2_pos;
		light_camera->lookAt(p2 + Vector3(0.0f, 0.25f, 0.0f), p2 + Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f));
		light_camera->setOrthographic(-1.0f, 1.0f, -1.0f, 1.0f, 0.05f, 15.0f);
		world->player2_vp = light_camera->viewprojection_matrix;
		world->player2_shadow_map = player2_shadow_fbo->depth_texture;

		player2_shadow_fbo->bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		world->render(light_camera);
		player2_shadow_fbo->unbind();
	}

	world->is_shadow_pass = false;
}

void Game::renderScene(Camera* camera)
{
	camera->enable();
	//This renders all meshes of the world
	if (skybox_tex && skybox_mesh && skybox_shader) {
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);

		Matrix44 sky_model;
		sky_model.setTranslation(camera->eye.x, camera->eye.y, camera->eye.z);
		sky_model.scale(10.0f, 10.0f, 10.0f);

		skybox_shader->enable();
		skybox_shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
		skybox_shader->setUniform("u_model", sky_model);
		skybox_shader->setTexture("u_texture", skybox_tex, 0);

		skybox_mesh->render(GL_TRIANGLES);

		skybox_shader->disable();

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}
	World::getInstance()->render(camera);
	drawGrid();
}

void Game::render(void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (game_state == TITLE)
	{
		glViewport(0, 0, window_width, window_height);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, window_width, window_height, 0, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glUseProgram(0);

		start_button->render();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);

		glEnable(GL_DEPTH_TEST);
	}
	else if (game_state == BATTLE)
	{
		World* world = World::getInstance();
		world->has_player1 = (player1 != nullptr);
		if (player1) world->player1_pos = player1->model.getTranslation();
		world->has_player2 = (player2 != nullptr);
		if (player2) world->player2_pos = player2->model.getTranslation();

		renderShadowMap();

		int viewport_width = (window_width - LINE_WIDTH) / 2;

		glViewport(0, 0, viewport_width, window_height);
		renderScene(camera1);

		glViewport(viewport_width + LINE_WIDTH, 0, viewport_width, window_height);
		renderScene(camera2);

		glViewport(0, 0, window_width, window_height);
		renderHUD();
	}
	else if (game_state == VICTORY_SCREEN) {
		glViewport(0, 0, window_width, window_height);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, window_width, window_height, 0, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glUseProgram(0);

		replay_button->render();
		return_title_button->render();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);

		glEnable(GL_DEPTH_TEST);
	}

	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{

	if (game_state != BATTLE)
		return;

	seconds_elapsed = std::min(seconds_elapsed, 0.05);

	float speed = (float)seconds_elapsed * mouse_speed;
	
	World::getInstance()->update((float)seconds_elapsed);

	checkVictory();

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
			float sensitivity = 0.1f;
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

		if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) {
			player1->jump(seconds_elapsed);
			player1->jump_pressed = true;
		}
		else {
			player1->stopJump();
			player1->jump_pressed = false;
		}

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


			if (Input::gamepads[0].button[A_BUTTON]) {
				player2->jump(seconds_elapsed);
				player2->jump_pressed = true;
			}
			else {
				player2->stopJump();
				player2->jump_pressed = false;
			};
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
	case SDLK_LSHIFT:
		if (camera_mode == THIRD_PERSON) {
			player1->dash();
			break;
		}
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event) {}

void Game::onMouseButtonDown(SDL_MouseButtonEvent event)
{
	if (game_state == TITLE) {
		if (event.button == SDL_BUTTON_LEFT) {
			if (start_button->is_clicked(event)){
				game_state = BATTLE;
				initBattle();
				setMouseLocked(true);
			}
		}

		return;
	}
	else if (game_state == VICTORY_SCREEN) {
		if (event.button == SDL_BUTTON_LEFT) {
			if (replay_button->is_clicked(event)) {
				winner = NO_WINNER;
				game_state = BATTLE;
				initBattle();
				setMouseLocked(true);
			}

			if (return_title_button->is_clicked(event)) {
				winner = NO_WINNER;
				game_state = TITLE;
				initTitle();
				setMouseLocked(false);
			}
		}
	}

	switch (event.button) {
	case SDL_BUTTON_MIDDLE:
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
		SDL_SetRelativeMouseMode((SDL_bool)(mouse_locked));
		break;
	case SDL_BUTTON_LEFT:
		player1->punch(); break;
	case SDL_BUTTON_RIGHT:
		player1->punch(); break;
	}
	
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event) {}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	mouse_speed *= event.y > 0 ? 1.1f : 0.9f;
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event) {
	switch (event.button)
	{
	case B_BUTTON:
		if (camera_mode == THIRD_PERSON) {
			player2->dash();
			break;
		}
	case X_BUTTON:
		if (camera_mode == THIRD_PERSON) {
			player2->punch();
			break;
		}
	case Y_BUTTON:
		if (camera_mode == THIRD_PERSON) {
			player2->punch();
			break;
		}
	}
}
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

void Game::checkVictory() {
	if (player1->current_hp <= 0) {
		winner = PLAYER_2;
	}
	else if (player2->current_hp <= 0) {
		winner = PLAYER_1;
	}
	else {
		return;
	}

	game_state = VICTORY_SCREEN;

	setMouseLocked(false);
	initVictoryScreen();
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
	float alpha, bool align_right)
{
	if (clip_w <= 0.0f || full_w <= 0.0f || h <= 0.0f) return;
	if (clip_w > full_w) clip_w = full_w;

	float x_start, x_end;
	float r_start, g_start, b_start;
	float r_end, g_end, b_end;

	if (!align_right) {
		// Anclada a la izquierda: se vacía de derecha a izquierda
		float t = clip_w / full_w;
		x_start = x;
		x_end = x + clip_w;

		r_start = r_left;
		g_start = g_left;
		b_start = b_left;

		r_end = r_left + (r_right - r_left) * t;
		g_end = g_left + (g_right - g_left) * t;
		b_end = b_left + (b_right - b_left) * t;
	}
	else {
		// Anclada a la derecha (centro): se vacía de izquierda a derecha
		float t = (full_w - clip_w) / full_w;
		x_start = x + (full_w - clip_w);
		x_end = x + full_w;

		r_start = r_left + (r_right - r_left) * t;
		g_start = g_left + (g_right - g_left) * t;
		b_start = b_left + (b_right - b_left) * t;

		r_end = r_right;
		g_end = g_right;
		b_end = b_right;
	}

	glBegin(GL_QUADS);
	glColor4f(r_start, g_start, b_start, alpha);
	glVertex2f(x_start, y);
	glVertex2f(x_start, y + h);

	glColor4f(r_end, g_end, b_end, alpha);
	glVertex2f(x_end, y + h);
	glVertex2f(x_end, y);
	glEnd();
}

void Game::renderHealthBar(Player* player, float x, float y, float width, float height, bool mirror)
{
	if (!player) return;

	// Mantiene el efecto de vibración al recibir daño
	if (player->flash_timer > 0.0f) {
		x += sinf(player->flash_timer * 90.0f) * 2.0f;
	}

	float hp_ratio = std::max(0.0f, std::min(1.0f, player->current_hp / player->max_hp));
	float ghost_ratio = std::max(0.0f, std::min(1.0f, player->ghost_hp / player->max_hp));

	// Capa 1: Sombra paralela
	drawClippedBar(x, y + (height * 0.12f), width, height, width,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.28f, mirror);

	// Capa 2: Fondo vacío oscuro (contrasta con la vida, el flash y la barra fantasma)
	float bg_rl = 0.18f, bg_gl = 0.04f, bg_bl = 0.04f;
	float bg_rr = 0.28f, bg_gr = 0.06f, bg_br = 0.06f;
	if (mirror) {
		std::swap(bg_rl, bg_rr);
		std::swap(bg_gl, bg_gr);
		std::swap(bg_bl, bg_br);
	}
	drawClippedBar(x, y, width, height, width,
		bg_rl, bg_gl, bg_bl,
		bg_rr, bg_gr, bg_br,
		0.85f, mirror);

	// Capa 3: Barra fantasma de daño retardado
	if (ghost_ratio > hp_ratio) {
		float gh_rl = 1.0f, gh_gl = 0.50f, gh_bl = 0.50f;
		float gh_rr = 1.0f, gh_gr = 0.65f, gh_br = 0.65f;
		if (mirror) {
			std::swap(gh_rl, gh_rr);
			std::swap(gh_gl, gh_gr);
			std::swap(gh_bl, gh_br);
		}
		drawClippedBar(x, y, width, height, width * ghost_ratio,
			gh_rl, gh_gl, gh_bl,
			gh_rr, gh_gr, gh_br,
			0.75f, mirror);
	}

	// Capa 4: Barra de vida principal (siempre visible sobre el fondo oscuro)
	float r_l = 0.82f, g_l = 0.02f, b_l = 0.02f;
	float r_r = 0.98f, g_r = 0.25f, b_r = 0.25f;

	if (player->flash_timer > 0.0f) {
		// Destello blanco/coral intenso que destaca tanto sobre el fondo oscuro como sobre el daño fantasma
		r_l = 1.0f; g_l = 0.85f; b_l = 0.85f;
		r_r = 1.0f; g_r = 1.0f;  b_r = 1.0f;
	}

	if (mirror) {
		std::swap(r_l, r_r);
		std::swap(g_l, g_r);
		std::swap(b_l, b_r);
	}

	drawClippedBar(x, y, width, height, width * hp_ratio,
		r_l, g_l, b_l,
		r_r, g_r, b_r,
		1.0f, mirror);
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

	renderHealthBar(player1, margin, margin, bar_w, bar_h, true);               // Espejada: degradado y vaciado hacia el centro
	renderHealthBar(player2, viewport_w + margin, margin, bar_w, bar_h, false); // Normal: degradado y vaciado hacia el centro

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}