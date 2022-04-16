using namespace std;
#include "Angel.h"
#include <string>

typedef vec4  color4;
typedef vec4  point4;


float cube_spacing = 0.5;

enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

const int NumCubes = 8;
const int NumCubeFaces = 6;
const int NumFaceVertices = 6;
const int singleCubeVertices = NumFaceVertices * NumCubeFaces; // 6 * 6 = 36 for each cube
const int NumVertices = singleCubeVertices * NumCubes; // 36 * 8 = 288 for all 8 cubes

// Animation speed
const int animation_speed = 8;

// Default view aspect of the cube
double aspectX = 135.0;
double aspectZ = -45.0;
double scaleFactor = 1.0;

mat4 rotationMatrix[NumCubes];
color4 back_buffer_colors[NumCubes];
color4 color_cycle[8] = {
	color4(0.0, 0.0, 0.5, 1.0), 
	color4(0.0, 0.5, 0.0, 1.0),  
	color4(0.5, 0.0, 0.0, 1.0), 
	color4(0.0, 0.5, 0.5, 1.0), 
	color4(0.5, 0.5, 0.5, 1.0), 
	color4(0.5, 0.0, 0.5, 1.0), 
	color4(0.5, 0.5, 0.0, 1.0),  
	color4(0.25, 0.5, 0.0, 1.0),  
};
color4 colors[NumVertices];
point4 vertices[NumVertices];

// Cube position
int current_cube_index[2][2][2];
int nextCubePos[2][2][2];

// Cube rotation variables
int rotationAxis = Yaxis;
int currentBlock = 1;
int rotateAngle = 0;

// Uniform location
int ModelView;
int Projection;
int edge;
int currentCube;
int vRotation[NumCubes];

// Variables for the shader
int selectedCube;
int selectedFace;
int cubeColor[NumCubes];

void create_cubes(point4 pointsCube[singleCubeVertices], color4 color[singleCubeVertices]) {
	point4 verticesCube[8];

	verticesCube[0] = point4(-cube_spacing, cube_spacing, -cube_spacing, 1.0);
	verticesCube[1] = point4(-cube_spacing, cube_spacing, cube_spacing, 1.0);
	verticesCube[2] = point4(cube_spacing, cube_spacing, cube_spacing, 1.0);
	verticesCube[3] = point4(cube_spacing, cube_spacing, -cube_spacing, 1.0);
	verticesCube[4] = point4(-cube_spacing, -cube_spacing, -cube_spacing, 1.0);
	verticesCube[5] = point4(-cube_spacing, -cube_spacing, cube_spacing, 1.0);
	verticesCube[6] = point4(cube_spacing, -cube_spacing, cube_spacing, 1.0);
	verticesCube[7] = point4(cube_spacing, -cube_spacing, -cube_spacing, 1.0);

	for (int i = 0; i < 6; i++) {
		color[i] = color4(1.0, 0.0, 0.0, 1.0);     // red
		color[i + 6] = color4(1.0, 1.0, 0.0, 1.0);   // yellow
		color[i + 12] = color4(0.0, 1.0, 0.0, 1.0);  // green
		color[i + 18] = color4(0.0, 1.0, 1.0, 1.0);  // cyan
		color[i + 24] = color4(0.0, 0.0, 1.0, 1.0);  // blue
		color[i + 30] = color4(1.0, 0.0, 1.0, 1.0);  // magenta
	}

	pointsCube[0] = verticesCube[0];
	pointsCube[1] = verticesCube[1];
	pointsCube[2] = verticesCube[2];
	pointsCube[3] = verticesCube[2];
	pointsCube[4] = verticesCube[3];
	pointsCube[5] = verticesCube[0];

	pointsCube[6] = verticesCube[4];
	pointsCube[7] = verticesCube[5];
	pointsCube[8] = verticesCube[6];
	pointsCube[9] = verticesCube[6];
	pointsCube[10] = verticesCube[7];
	pointsCube[11] = verticesCube[4];

	pointsCube[12] = verticesCube[0];
	pointsCube[13] = verticesCube[3];
	pointsCube[14] = verticesCube[7];
	pointsCube[15] = verticesCube[7];
	pointsCube[16] = verticesCube[4];
	pointsCube[17] = verticesCube[0];

	pointsCube[18] = verticesCube[1];
	pointsCube[19] = verticesCube[2];
	pointsCube[20] = verticesCube[6];
	pointsCube[21] = verticesCube[6];
	pointsCube[22] = verticesCube[5];
	pointsCube[23] = verticesCube[1];

	pointsCube[24] = verticesCube[0];
	pointsCube[25] = verticesCube[1];
	pointsCube[26] = verticesCube[5];
	pointsCube[27] = verticesCube[5];
	pointsCube[28] = verticesCube[4];
	pointsCube[29] = verticesCube[0];

	pointsCube[30] = verticesCube[3];
	pointsCube[31] = verticesCube[2];
	pointsCube[32] = verticesCube[6];
	pointsCube[33] = verticesCube[6];
	pointsCube[34] = verticesCube[7];
	pointsCube[35] = verticesCube[3];
}

void rubiksCube() {
	int current = 0;
	float red = 0.0;
	float green = 0.0;
	float blue = 0.0;
	point4 cube[singleCubeVertices];
	color4 color[singleCubeVertices];

	// Create 8 sub cubes
	int color_cycle_iterator = 0;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				create_cubes(cube, color);

				// Cube translation
				mat4 translation = Translate(vec3(i - 0.5, j - 0.5, k - 0.5));
				for (int i = 0; i < singleCubeVertices; i++) {
					cube[i] = translation * cube[i];
				}

				// Set vertices and colors
				for (int m = 0; m < singleCubeVertices; m++) {
					vertices[current * singleCubeVertices + m] = cube[m];
					colors[current * singleCubeVertices + m] = color[m];
				}
				nextCubePos[i][j][k] = current;
				current_cube_index[i][j][k] = current;

				back_buffer_colors[current] = color_cycle[current];
				current++;
			}
		}
	}
}


void rotateCube(int angle) {
	int currentPos = 0;

	// Update rotation matrix
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			if (rotationAxis == Xaxis) {
				currentPos = current_cube_index[currentBlock][i][j];
				rotationMatrix[currentPos] = RotateX(angle) * rotationMatrix[currentPos];
			}
			else if (rotationAxis == Yaxis) {
				currentPos = current_cube_index[i][currentBlock][j];
				rotationMatrix[currentPos] = RotateY(angle) * rotationMatrix[currentPos];
			}
			else if (rotationAxis == Zaxis) {
				currentPos = current_cube_index[i][j][currentBlock];
				rotationMatrix[currentPos] = RotateZ(angle) * rotationMatrix[currentPos];
			}
			glUniformMatrix4fv(vRotation[currentPos], 1, GL_TRUE, rotationMatrix[currentPos]);
		}
	}

	rotateAngle = rotateAngle + angle;
	glutPostRedisplay();
	if (rotateAngle == 90) {
		for (int k = 0; k < 2; k++) {
			for (int l = 0; l < 2; l++) {
				for (int m = 0; m < 2; m++) {
					current_cube_index[k][l][m] = nextCubePos[k][l][m];
				}
			}
		}
		rotateAngle = 0;
	}
	else {
		glutTimerFunc(animation_speed, rotateCube, angle);
	}
}

void setNextPositions(int direction, int subcube, int axis) {
	int rotation[2][2];
	if (direction < 0) {
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				if (axis == Xaxis) {
					rotation[j][1 - i] = nextCubePos[subcube][i][j];
				}
				else if (axis == Yaxis) {
					rotation[1 - j][i] = nextCubePos[i][subcube][j];
				}
				else if (axis == Zaxis) {
					rotation[j][1 - i] = nextCubePos[i][j][subcube];
				}
			}
		}
	}
	else {
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				if (axis == Xaxis) {
					rotation[1 - j][i] = nextCubePos[subcube][i][j];
				}
				else if (axis == Yaxis) {
					rotation[j][1 - i] = nextCubePos[i][subcube][j];
				}
				else if (axis == Zaxis) {
					rotation[1 - j][i] = nextCubePos[i][j][subcube];
				}
			}
		}
	}

	// Copy subcube back to nextCubePos
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			if (axis == Xaxis) {
				nextCubePos[subcube][i][j] = rotation[i][j];
			}
			else if (axis == Yaxis) {
				nextCubePos[i][subcube][j] = rotation[i][j];
			}
			else if (axis == Zaxis) {
				nextCubePos[i][j][subcube] = rotation[i][j];
			}
		}
	}
}


int setParameters(int axis, int firstAxis, int secondAxis, int key) {
	int direction = 0;

	// Rotate on X axis
	if (axis == Xaxis) {
		if (secondAxis == 0) {
			currentBlock = 0;
			direction = -1;
			if (firstAxis == 0) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Zaxis;
					direction = 1;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					rotationAxis = Yaxis;
				}
			}
			else if (firstAxis == 1) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Zaxis;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					currentBlock = 1;
					rotationAxis = Yaxis;
				}
			}
		}
		else if (secondAxis == 1) {
			currentBlock = 1;
			direction = 1;
			if (firstAxis == 0) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Zaxis;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					currentBlock = 0;
					rotationAxis = Yaxis;
				}
			}
			else if (firstAxis == 1) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Zaxis;
					direction = -1;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					rotationAxis = Yaxis;
				}
			}
		}
		// Rotate on Y axis
	}
	else if (axis == Yaxis) {
		if (firstAxis == 0) {
			direction = 1;
			currentBlock = 0;
			if (secondAxis == 0) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Zaxis;
					direction = -1;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					rotationAxis = Xaxis;
				}
			}
			else if (secondAxis == 1) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Zaxis;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					currentBlock = 1;
					rotationAxis = Xaxis;
				}
			}
		}
		else if (firstAxis == 1) {
			direction = -1;
			currentBlock = 1;
			if (secondAxis == 0) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Zaxis;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					currentBlock = 0;
					rotationAxis = Xaxis;
				}
			}
			else if (secondAxis == 1) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Zaxis;
					direction = 1;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					rotationAxis = Xaxis;
				}
			}
		}
		// Rotate on Z axis
	}
	else if (axis == Zaxis) {
		if (firstAxis == 0) {
			currentBlock = 0;
			direction = 1;
			if (secondAxis == 0) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Xaxis;
					direction = -1;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					rotationAxis = Yaxis;
				}
			}
			else if (secondAxis == 1) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Xaxis;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					currentBlock = 1;
					rotationAxis = Yaxis;
				}
			}
		}
		else if (firstAxis == 1) {
			direction = -1;
			currentBlock = 1;
			if (secondAxis == 0) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Xaxis;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					currentBlock = 0;
					rotationAxis = Yaxis;
				}
			}
			else if (secondAxis == 1) {
				if (key == GLUT_LEFT_BUTTON) {
					rotationAxis = Xaxis;
					direction = 1;
				}
				else if (key == GLUT_RIGHT_BUTTON) {
					rotationAxis = Yaxis;
				}
			}
		}
	}
	return direction;
}


int getDirection(int currentX, int currentY, int currentZ, int nextX, int nextY, int nextZ, int key) {
	int direction = 0;
	if (currentX == 0) {
		if (currentY == 0) {
			if (currentZ == 1) {
				direction = setParameters(Xaxis, nextY, nextZ, key);
			}
		}
		else if (currentY == 1) {
			direction = setParameters(Zaxis, nextX, nextY, key);
			if (currentZ == 1) {
				direction = direction * -1;
			}
		}
	}
	else if (currentX == 1) {
		if (currentY == 0) {
			if (currentZ == 0) {
				direction = setParameters(Yaxis, nextZ, nextX, key);
			}
			else if (currentZ == 1) {
				direction = setParameters(Xaxis, nextY, nextZ, key);
			}
			direction = direction * -1;
		}
		else if (currentY == 1) {
			if (currentZ == 0) {
				direction = setParameters(Yaxis, nextZ, nextX, key);
			}
		}
	}
	return direction;
}


int cubeSelector(int x, int y, int key) {
	int cube_index = -1;
	int direction = 0;
	int temp = 0;
	int nextX = -1;
	int nextY = -1;
	int nextZ = -1;
	unsigned char point[3];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUniform1i(selectedCube, 1);

	for (int i = 0; i < NumVertices; i++) {
		glUniform1i(currentCube, i);
		glDrawArrays(GL_TRIANGLES, temp, singleCubeVertices);
		temp = temp + singleCubeVertices;
	}

	glUniform1i(selectedCube, 0);
	glFlush();
	y = glutGet(GLUT_WINDOW_HEIGHT) - y;
	// Get the cube that is clicked.
	glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, point);
	GLfloat k = double(point[0])/256.0;
	GLfloat l = double(point[1]) / 256.0;
	GLfloat m = double(point[2]) / 256.0;
	for (int i = 0; i < 8; i++) {
		color4 current_color = color_cycle[i];
		if (current_color.x == k && current_color.y == l && current_color.z == m) {
			cube_index = i;
			break;
		}
	}
	if (cube_index != -1) {

		//cube_index = (k * 4) + (l * 2) + m;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUniform1i(selectedFace, 1);

		temp = 0;
		for (int i = 0; i < NumVertices; i++) {
			glUniform1i(currentCube, i);
			glDrawArrays(GL_TRIANGLES, temp, singleCubeVertices);
			temp = temp + singleCubeVertices;
		}

		glUniform1i(selectedFace, 0);
		glFlush();
		// Get the face that is clicked.
		glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, point);

		k = (int)point[0];
		l = (int)point[1];
		m = (int)point[2];
		k = ceil(k / 255.0);
		l = ceil(l / 255.0);
		m = ceil(m / 255.0);

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				for (int k = 0; k < 2; k++) {
					if (current_cube_index[i][j][k] == cube_index) {
						nextX = i; nextY = j; nextZ = k;
						break;
					}
				}
			}
		}

		// Rotate with appropriate axes
		direction = getDirection(k, l, m, nextX, nextY, nextZ, key);
		glutPostRedisplay();
	}


	return direction;
}


void randomizeCube(int a) {
	int direction;

	//srand((unsigned int)time(NULL));
	currentBlock = rand() % 2;
	rotationAxis = rand() % 3;

	if (rand() % 2 == 1) {
		direction = 1;
		setNextPositions(direction, currentBlock, rotationAxis);
		rotateAngle = 0;
		rotateCube(5);
	}
	else {
		direction = -1;
		setNextPositions(direction, currentBlock, rotationAxis);
		rotateAngle = 180;
		rotateCube(-5);
	}
}


void init() {
	// Rubik's cube init
	rubiksCube();

	// Initialize rotation matrix
	for (int i = 0; i < NumCubes; i++) {
		rotationMatrix[i] = RotateX(0);
	}

	// Randomize Rubik's cube
	glutTimerFunc(4300, randomizeCube, 0);
	glutTimerFunc(3600, randomizeCube, 0);
	glutTimerFunc(2900, randomizeCube, 0);
	glutTimerFunc(2200, randomizeCube, 0);
	glutTimerFunc(1500, randomizeCube, 0);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	// Add vertices and colors to buffer
	glBufferData(GL_ARRAY_BUFFER, (sizeof(point4) * NumVertices * 2), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * NumVertices, vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * NumVertices, sizeof(color4) * NumVertices, colors);

	//Load shaders and use the resulting shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");

	// Set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint svPosition = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(svPosition);
	glVertexAttribPointer(svPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point4) * NumVertices));

	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");
	currentCube = glGetUniformLocation(program, "currentCube");
	edge = glGetUniformLocation(program, "edge");
	selectedCube = glGetUniformLocation(program, "selectedCube");
	selectedFace = glGetUniformLocation(program, "selectedFace");

	// Send cube to the shader
	for(int i = 0; i < NumCubes; i++)
	{
		vRotation[i] = glGetUniformLocation(program, ("vRotation[" + to_string(i) + "]").c_str());
		glUniformMatrix4fv(vRotation[i], 1, GL_TRUE, rotationMatrix[i]);
		cubeColor[i] = glGetUniformLocation(program, ("cubeColor[" + to_string(i) + "]").c_str());
		glUniform4fv(cubeColor[i], 1, back_buffer_colors[i]);
	}

	glUseProgram(program);
	glEnable(GL_DEPTH_TEST | GL_LINE_SMOOTH);
	glLineWidth(4.0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const vec3 displacement(0, 0, 0);
	mat4  model_view = (Translate(displacement) *
		RotateX(Theta[Xaxis] + aspectX) *
		RotateY(Theta[Yaxis]) *
		RotateZ(Theta[Zaxis] + aspectZ));

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

	// Render subcubes
	int cubeVertices = 0;
	for(int i = 0; i < NumVertices; i++) {
		glUniform1i(currentCube, i);
		glDrawArrays(GL_TRIANGLES, cubeVertices, singleCubeVertices);
		cubeVertices += singleCubeVertices;
	}

	glUniform1i(edge, 1);

	// Render cube edges
	int temp = -1;
	cubeVertices = 0;
	while (cubeVertices < NumVertices) {
		if (cubeVertices % singleCubeVertices == 0) {
			temp++;
		}
		glUniform1i(currentCube, temp);
		glDrawArrays(GL_LINE_STRIP, cubeVertices, 3);
		cubeVertices += 3;
	}

	glUniform1i(edge, 0);
	glutSwapBuffers();
}


void reshape(int width, int height) {
	glViewport(0, 0, width, height);

	// Set the projection matrix
	mat4 projection;

	if (width <= height)
		projection = Ortho(-4, 4, -4 * ((GLfloat)height / width), 4 * ((GLfloat)height / width), -4, 4);
	else
		projection = Ortho(-4 * ((GLfloat)width / height), 4 * ((GLfloat)width / height), -4, 4, -4, 4);

	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
}


void helpMenu() {
	printf("-h or H for help\n");
	printf("-q or Q to exit\n\n");
	printf("-i will navigate to the initial angle of the rubiks cube. \n");
	printf("-r: Randomize the rubiks cube\n");
	printf("To control the rubiks cube angle, use keys w, a, s, d. \n");
	printf("Click to the cubes with mouse to rotate a face.\n");
	printf(" 1. For a vertical rotation, perform a right-click\n");
	printf(" 2. For a horizontal rotation, perform a left-click\n");
	
}

void mouse(int key, int state, int x, int y) {
	int direction;

	if (state == GLUT_DOWN && rotateAngle == 0) {
		// Set the current block, rotation axis and direction
		direction = cubeSelector(x, y, key);
		if (direction == -1) {
			setNextPositions(direction, currentBlock, rotationAxis);
			rotateAngle = 180;
			rotateCube(-5);
		}
		else if (direction == 1) {
			setNextPositions(direction, currentBlock, rotationAxis);
			rotateAngle = 0;
			rotateCube(5);
		}
	}
}


void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'i': case 'I':
		aspectX = 135.0;
		aspectZ = -45.0;
		break;
	case 'r': case 'R':
		randomizeCube(0);
		glutTimerFunc(2400, randomizeCube, 0);
		glutTimerFunc(1800, randomizeCube, 0);
		glutTimerFunc(1200, randomizeCube, 0);
		glutTimerFunc(600, randomizeCube, 0);

		// break;
	case 'h': case 'H':
		helpMenu();
		break;
	case 'q': case 'Q':
		exit(0);
		break;
	case 'a': case 'A':
		aspectZ -= 5;
		break;
	case 'd': case 'D':
		aspectZ += 5;
		break;
	case 'w': case 'W':
		aspectX += 5;
		break;
	case 's': case 'S':
		aspectX -= 5;
		break;
	}
	glutPostRedisplay();
}


int main(int agrc, char** agrv) {
	glutInit(&agrc, agrv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(200, 50);
	glutCreateWindow("Rubiks Cube");
	glewExperimental = GL_TRUE;
	glewInit();
	init();
	helpMenu();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);

	glutMainLoop();
	return 0;
}