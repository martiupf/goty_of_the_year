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
#include "framework/entities/entity.h"

enum CameraMode {
	FREE_CAMERA,
	THIRD_PERSON
};

class Game
{
public:
	static Game* instance;

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
	Player* player1;
	float cam1_yaw = 0.0f;
	float cam1_pitch = 0.5f;
	Player* player2;
	float cam2_yaw = 0.0f;
	float cam2_pitch = 0.5f;
	CameraMode camera_mode;

	//some vars
	Camera* camera1;
	Camera* camera2;
	bool mouse_locked; //tells if the mouse is locked (not seen)

	Game( int window_width, int window_height, SDL_Window* window );

	//main functions
	void render( void );
	void renderScene(Camera* camera);
	void update( double dt );

	bool loadMap(const char* scene_filepath);

	void setMouseLocked(bool must_lock);

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