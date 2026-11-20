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

enum GameState {
	TITLE,
	MAP_SELECT,
	BATTLE,
	VICTORY_SCREEN
};


class Button
{
public:
	float button_x;
	float button_y;
	float button_width;
	float button_height;

	GLuint texture_id;

	Button() {
		button_x = 0;
		button_y = 0;
		button_width = 0;
		button_height = 0;
		texture_id = 0;
	}

	void render()
	{
		if (texture_id == 0)
			return;

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindTexture(GL_TEXTURE_2D, texture_id);

		glColor4f(1, 1, 1, 1);

		glBegin(GL_QUADS);

		glTexCoord2f(0, 0);
		glVertex2f(button_x, button_y);

		glTexCoord2f(1, 0);
		glVertex2f(button_x + button_width, button_y);

		glTexCoord2f(1, 1);
		glVertex2f(button_x + button_width, button_y + button_height);

		glTexCoord2f(0, 1);
		glVertex2f(button_x, button_y + button_height);

		glEnd();

		glBindTexture(GL_TEXTURE_2D, 0);

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
};

GLuint loadTexture(const char* filename);

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
	GameState game_state = BATTLE;

	Camera* camera1 = nullptr;
	Camera* camera2 = nullptr;
	bool mouse_locked; //tells if the mouse is locked (not seen)

	// buttons
	Button start_button;


	Game( int window_width, int window_height, SDL_Window* window );

	//main functions
	void render( void );
	void renderScene(Camera* camera);
	void renderHUD();
	void update( double dt );

	//game_state flow
	void initMenu();
	void initBattle();

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
