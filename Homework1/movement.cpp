#include "Angel.h"

struct movement {

	GLfloat locX;
	GLfloat locY;
	GLfloat velocityX;
	GLfloat velocityY;

	void initMovement(GLfloat locX, GLfloat locY, GLfloat velocityX, GLfloat velocityY) {
		this->locX = locX;
		this->locY = locY;
		this->velocityX = velocityX;
		this->velocityY = velocityY;
	}


};