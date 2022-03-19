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
        

        if ((locY >= 1 - radiusY || locY <= -1 + radiusY)) {
            velocityY *= -1.0;
        }

        if ((locX >= 1 - radiusX || locX <= -1 + radiusX)) {
            velocityX *= -1.0;
        }

        locX += velocityX;
        locY += velocityY;

        
	}


};