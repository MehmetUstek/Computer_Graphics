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

	void updateObjectLocation() {
        int radiusX = 1;
        int radiusY = 1;
        int width = 512;
        int height = 380;

        // y-axis floor collision check
        if (locY <= -10.0 + radiusY) {
            velocityY *= -0.9;
            velocityX *= 0.8;

        }

        // x-axis wall collision check
        if ((locX >= width - radiusX || locX <= -width + radiusX)) {
            velocityX *= -1.0;
        }

        locX += velocityX;
        locY += velocityY * 10 * radiusY;

        
	}


};