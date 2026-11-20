#include "button.h"

Button::Button(float x, float y, float width, float height, Texture* texture) {
	button_x = x;
	button_y = y;
	button_width = width;
	button_height = height;
	this->texture = texture;
}

void Button::render() {
	if (!texture)
		return;

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	texture->bind();
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

	texture->unbind();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

bool Button::is_clicked(SDL_MouseButtonEvent event) {
	if (event.x >= button_x &&
		event.x <= button_x + button_width &&
		event.y >= button_y &&
		event.y <= button_y + button_height) {

		return true;
	}
	else {
		return false;
	}
}