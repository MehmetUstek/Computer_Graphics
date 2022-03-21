#include "Angel.h"

struct ObjectLocation {

	GLfloat locX, locY, velocityX, velocityY;
	float projectionConstant = 5.0f;

	void initObjectLocation(GLfloat locX, GLfloat locY, GLfloat velocityX, GLfloat velocityY, float projection_constant) {
		this->locX = locX;
		this->locY = locY;
		this->velocityX = velocityX;
		this->velocityY = velocityY;
	}

	void updateObjectLocation(GLfloat radiusX, GLfloat radiusY, GLfloat widthRatio, GLfloat heightRatio) {
        

        if ((locY >= projectionConstant* heightRatio - radiusY || locY <= -projectionConstant * heightRatio + radiusY)) {
            velocityY *= -1.0;
        }

        if ((locX >= projectionConstant* widthRatio - radiusX || locX <= -projectionConstant * widthRatio + radiusX)) {
            velocityX *= -1.0;
        }

        locX += velocityX;
        locY += velocityY;

        
	}


};