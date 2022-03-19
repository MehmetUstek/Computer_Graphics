#include "Angel.h"

struct ObjectLocation {

	GLfloat locX, locY, velocityX, velocityY, v_init_X, v_init_Y;

	void initObjectLocation(GLfloat locX, GLfloat locY, GLfloat velocityX, GLfloat velocityY) {
		this->locX = locX;
		this->locY = locY;
		this->velocityX = velocityX;
		this->velocityY = velocityY;
		this->v_init_X = velocityX;
		this->v_init_Y = velocityY;
	}

	void updateObjectLocation(GLfloat radiusX, GLfloat radiusY) {
        GLsizei width = 512;
        GLsizei height = 380;

        // y-axis floor collision check
        if (locY <= -1 + radiusY) {
            velocityY *= -1;
            velocityX *= 0.8;

        }

        // x-axis wall collision check
        if ((locX >= 1 - radiusX || locX <= -1 + radiusX)) {
            velocityX *= -1.0;
        }

        locX += velocityX;
        locY += velocityY * radiusY;

        
	}


};