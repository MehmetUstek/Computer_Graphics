#include "Angel.h"

struct ObjectLocation {

	GLfloat locX, locY, velocityX, velocityY, v_init_X, v_init_Y;
	float projectionConstant = 5;

	void initObjectLocation(GLfloat locX, GLfloat locY, GLfloat velocityX, GLfloat velocityY, float projection_constant) {
		this->locX = locX;
		this->locY = locY;
		this->velocityX = velocityX;
		this->velocityY = velocityY;
		this->v_init_X = velocityX;
		this->v_init_Y = velocityY;
	}

	void updateObjectLocation(GLfloat radiusX, GLfloat radiusY) {
        

        if ((locY >= projectionConstant - radiusY || locY <= -projectionConstant + radiusY)) {
            velocityY *= -1.0;
        }

        if ((locX >= projectionConstant - radiusX || locX <= -projectionConstant + radiusX)) {
            velocityX *= -1.0;
        }

        locX += velocityX;
        locY += velocityY;

        
	}


};