#pragma once
#include "graphics/texture.h"

class Button
{
public:
	float button_x;
	float button_y;
	float button_width;
	float button_height;

	Texture* texture = nullptr;

	Button(float button_x, float button_y, float button_width, float button_height, Texture* texture);
	
	void render();
	bool is_clicked(SDL_MouseButtonEvent event);
};