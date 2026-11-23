/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#pragma once

#include "framework/includes.h"
#include "framework/camera.h"
#include "framework/utils.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"
#include "graphics/shader.h"
#include "framework/input.h"
#include "world.h"
#include "entityMesh.h"
#include "player.h"
#include "button.h"
#include "framework/entities/entity.h"

enum CameraMode {
	FREE_CAMERA,
	THIRD_PERSON
};

enum GameState {
	TITLE,
	MAP_SELECT,
	BATTLE,
	VICTORY_SCREEN
};

enum Winner {
	NO_WINNER,
	PLAYER_1,
	PLAYER_2
};

class Game
{
public:
	static Game* instance;

	Texture* skybox_tex = nullptr;
	Mesh* skybox_mesh = nullptr;
	Shader* skybox_shader = nullptr;

	//window
	SDL_Window* window;
	int window_width;
	int window_height;

	//some globals
	long frame;
    float time;
	float elapsed_time;
	int fps;
	bool must_exit;

	//n
	Player* player1 = nullptr;
	float cam1_yaw = 0.0f;
	float cam1_pitch = 0.5f;
	Player* player2 = nullptr;
	float cam2_yaw = 0.0f;
	float cam2_pitch = 0.5f;
	CameraMode camera_mode;

	//some vars
	GameState game_state = TITLE;

	Camera* camera1 = nullptr;
	Camera* camera2 = nullptr;
	FBO* shadow_fbo = nullptr;
	Camera* light_camera = nullptr;
	Vector3 shadow_box_min = Vector3(-35.0f, -10.0f, -35.0f);
	Vector3 shadow_box_max = Vector3(35.0f, 35.0f, 35.0f);
	bool mouse_locked; //tells if the mouse is locked (not seen)

	// buttons
	Button* start_button;
	Button* replay_button;
	Button* return_title_button;

	Game( int window_width, int window_height, SDL_Window* window );

	//main functions
	void render( void );
	void renderShadowMap(void);
	void renderScene(Camera* camera);
	void renderHUD();
	void update( double dt );

	//game_state flow
	void initTitle();
	void initMapSelect();
	void initBattle();
	void initVictoryScreen();
	void checkVictory();
	Winner winner = NO_WINNER;

	bool loadMap(const char* scene_filepath);

	void setMouseLocked(bool must_lock);

	float health_bar_height_ratio = 0.05f;
	void renderHealthBar(Player* player, float x, float y, float width, float height, bool mirror = false);
	void drawClippedBar(float x, float y, float full_w, float h, float clip_w,
		float r_left, float g_left, float b_left,
		float r_right, float g_right, float b_right,
		float alpha, bool align_right = false);

	//events
	void onKeyDown( SDL_KeyboardEvent event );
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown( SDL_MouseButtonEvent event );
	void onMouseButtonUp(SDL_MouseButtonEvent event);
	void onMouseWheel(SDL_MouseWheelEvent event);
	void onGamepadButtonDown(SDL_JoyButtonEvent event);
	void onGamepadButtonUp(SDL_JoyButtonEvent event);
	void onResize(int width, int height);

	void updateOrbitCamera();

	//n
	void updateThirdPersonCamera(Camera* camera, Player* player);

};
