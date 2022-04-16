//
//  Display a rotating cube, revisited
//

#include "Angel.h"

typedef vec4  color4;
typedef vec4  point4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
const int NumSquares = 8;
const float scaleConst = 4.0f;
const float squareWidth = 0.5f;
const float spacingBetweenCubes = squareWidth * 2 + 0.05;

point4 points[NumSquares + 1][NumVertices];
color4 colors[NumSquares + 1][NumVertices];
const int xoff = 1, yoff = 4, zoff = 16;


point4 cubePositions[8];
point4 currentRotation[8];
// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4(-squareWidth, -squareWidth,  squareWidth, 1.0),
    point4(-squareWidth,  squareWidth,  squareWidth, 1.0),
    point4(squareWidth,  squareWidth,  squareWidth, 1.0),
    point4(squareWidth, -squareWidth,  squareWidth, 1.0),
    point4(-squareWidth, -squareWidth, -squareWidth, 1.0),
    point4(-squareWidth,  squareWidth, -squareWidth, 1.0),
    point4(squareWidth,  squareWidth, -squareWidth, 1.0),
    point4(squareWidth, -squareWidth, -squareWidth, 1.0)
};

// RGBA olors
color4 vertex_colors[8] = {
    color4(0.0, 0.0, 0.0, 1.0),  // black
    color4(1.0, 0.0, 0.0, 1.0),  // red
    color4(1.0, 1.0, 0.0, 1.0),  // yellow
    color4(0.0, 0.0, 1.0, 1.0),  // blue
    color4(0.0, 1.0, 0.0, 1.0),  // green
    color4(1.0, 0.0, 1.0, 1.0),  // magenta
    color4(1.0, 1.0, 1.0, 1.0),  // white
    color4(0.0, 1.0, 1.0, 1.0)   // cyan
};
color4 color_cycle[6] = {
    color4(0.0, 0.0, 1.0, 1.0),  // blue
    color4(1.0, 0.0, 0.0, 1.0),  // red
    color4(1.0, 1.0, 0.0, 1.0),  // yellow
    color4(1.0, 0.0, 1.0, 1.0),  // magenta
    color4(0.0, 1.0, 0.0, 1.0),  // green
    color4(1.0, 0.5, 0.0, 1.0),  // orange
};

double angleX = 0.0;
double angleZ = 0.0;

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int  Axis = Zaxis;
GLfloat  Theta[NumAxes] = { 45.0, -45.0, 0.0 };

// Model-view and projection matrices uniform location
GLuint ModelView, Projection;
GLuint Color;

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices

int Index = 0;
int colorIndex = 0;

void
quad(int a, int b, int c, int d, int cubeIndex)
{
    color4 faceColor = color_cycle[colorIndex];
    colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[a]; Index++;
    colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[b]; Index++;
    colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[c]; Index++;
    colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[a]; Index++;
    colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[c]; Index++;
    colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[d]; Index++;
    colorIndex++;

}


//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube(bool isMiddleObject, int cubeIndex)
{
    quad(1, 0, 3, 2, cubeIndex);
    quad(2, 3, 7, 6, cubeIndex);
    quad(3, 0, 4, 7, cubeIndex);
    quad(3, 0, 4, 7, cubeIndex);
    quad(6, 5, 1, 2, cubeIndex);
    quad(4, 5, 6, 7, cubeIndex);
    quad(5, 4, 0, 1, cubeIndex);
    Index = 0;
    colorIndex = 0;
}

//----------------------------------------------------------------------------

// OpenGL initialization
bool topFaceClockwise = false; bool topFaceCounterClockwise = false;
bool rightFaceClockwise = false; bool rightFaceCounterClockwise = false;
bool leftFaceClockwise = false; bool leftFaceCounterClockwise = false;
bool turn_up = false; bool turn_down = false; bool turn_right = false; bool turn_left = false;

GLuint vao[NumSquares + 1];
GLuint vPosition, vColor;
GLuint buffer;
mat4 model_view[NumSquares + 1];
void
init()
{
    glGenVertexArrays(NumSquares, vao);
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    for (int i = 0; i < NumSquares + 1; i++) {
        colorcube(false, i);

        // Create a vertex array object


        glBindVertexArray(vao[i]);

        // Create and initialize a buffer object

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points[i]) + sizeof(colors[i]), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points[i]), points[i]);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(points[i]), sizeof(colors[i]), colors[i]);

        // Load shaders and use the resulting shader program


        // set up vertex arrays
        vPosition = glGetAttribLocation(program, "vPosition");
        glEnableVertexAttribArray(vPosition);
        glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        vColor = glGetAttribLocation(program, "vColor");
        glEnableVertexAttribArray(vColor);
        glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points[i])));

        model_view[i] = identity();
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view[i]);
    }
    float startingX = -spacingBetweenCubes;
    float startingY = -spacingBetweenCubes;
    float startingZ = -spacingBetweenCubes;
    float updatedY = startingY;
    for (int i = 0; i < NumSquares; i++) {
        glBindVertexArray(vao[i]);
        //  Generate tha model-view matrix
        vec3 displacement;

        if (i == 0) {
            displacement = vec3(startingX, startingY, startingZ);
        }
        else if (i % 2 == 0) {
            if (i % 4 == 0) {
                startingZ = startingZ - spacingBetweenCubes;
                updatedY = startingY - spacingBetweenCubes * float(Theta[Yaxis] / 135);
                startingX = startingX + float(Theta[Xaxis] / 90);
                displacement = vec3(startingX, updatedY, startingZ);

            }
            else {
                updatedY = updatedY + spacingBetweenCubes * -float(Theta[Yaxis] / 90);
                startingZ += 0.6;
                displacement = vec3(startingX, updatedY, startingZ);
            }

        }
        else {
            // 1,3,5,7 The cubes at right at creation.
            displacement = vec3(startingX + (spacingBetweenCubes * float(Theta[Xaxis] / 90)), updatedY + spacingBetweenCubes * float(Theta[Yaxis] / 135), startingZ + 0.6);
        }
        model_view[i] = (Translate(displacement) * Scale(1.0, 1.0, 1.0) *
            RotateX(Theta[Xaxis]) *
            RotateY(Theta[Yaxis]) *
            RotateZ(Theta[Zaxis]));
        cubePositions[i] = displacement;
    }


    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation(program, "ModelView");
    Projection = glGetUniformLocation(program, "Projection");

    mat4  projection;
    projection = Ortho(-scaleConst, scaleConst, -scaleConst, scaleConst, -scaleConst, scaleConst); // Ortho(): user-defined function in mat.h
    //projection = Perspective( 90.0, 3.0, 15.0, 3.0 ); //try also perspective projection instead of ortho
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

    // Set current program object
    glUseProgram(program);


    Color = glGetUniformLocation(program, "color");
    // Enable hiddden surface removal
    glEnable(GL_DEPTH_TEST);

    // Set state variable "clear color" to clear buffer with.
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

//----------------------------------------------------------------------------

void
display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float startingX = -spacingBetweenCubes;
    float startingY = -spacingBetweenCubes;
    float startingZ = -spacingBetweenCubes;
    float updatedY = startingY;
    for (int i = 0; i < NumSquares; i++) {
        glBindVertexArray(vao[i]);
        
        if (turn_down) {
            model_view[i] = RotateX(180.0) * model_view[i];
            std::cout << "Turn down" << std::endl;
        }
        if (turn_up) {
            model_view[i] = RotateX(180.0) * model_view[i];
            std::cout << "Turn up" << std::endl;
        }
        if (turn_right) {
            model_view[i] = RotateX(45.0) *  RotateY(-90.0) * RotateX(-45.0) * model_view[i];
            std::cout << "Turn up" << std::endl;
        }
        if (turn_left) {
            model_view[i] = RotateX(45.0) * RotateY(90.0) * RotateX(-45.0) * model_view[i];
            std::cout << "Turn up" << std::endl;
        }


        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view[i]);

        // Dont draw if it is the big cube. Draw only when mouse is clicked and immediately flush it so the user dont see.
        glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    }
    turn_down = false;
    turn_up = false;
    turn_right = false;
    turn_left = false;

    //TODO: Left point
    


    glutSwapBuffers();

}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    // Set projection matrix
    mat4  projection;
    if (w <= h)
        projection = Ortho(-scaleConst, scaleConst, -scaleConst * (GLfloat)h / (GLfloat)w,
            scaleConst * (GLfloat)h / (GLfloat)w, -scaleConst, scaleConst);
    else  projection = Ortho(-scaleConst * (GLfloat)w / (GLfloat)h, scaleConst *
        (GLfloat)w / (GLfloat)h, -scaleConst, scaleConst, -scaleConst, scaleConst);

    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

    //reshape callback needs to be changed if perspective prohection is used
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

void
keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'i': case 'I':
        angleX = 0.0;
        angleZ = 0.0;
        break;
    case 'h': case 'H':
        helpMenu();
        break;
    case 'q': case 'Q':
        exit(0);
        break;
    case 'a': case 'A':
        turn_left = true;
        break;
    case 'd': case 'D':
        turn_right = true;
        break;
    case 'w': case 'W':
        turn_up = true;
        break;
    case 's': case 'S':
        turn_down = true;
        break;
    }
    glutPostRedisplay();


}

//----------------------------------------------------------------------------

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(vao[8]);
        model_view[8] = (Translate(vec3(-spacingBetweenCubes / 2 + 0.1, -spacingBetweenCubes / 2, 0.8)) * Scale(2.0, 2.0, 1.0) *
            RotateX(Theta[Xaxis]) *
            RotateY(Theta[Yaxis]) *
            RotateZ(Theta[Zaxis]));
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view[8]);
        glDrawArrays(GL_TRIANGLES, 0, NumVertices);
        glFlush();

        y = glutGet(GLUT_WINDOW_HEIGHT) - y;
        unsigned char pixel[4];
        glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

        if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 255) {
            leftFaceCounterClockwise = true;
            std::cout << "LeftfaceCounterClockwise Clicked" << std::endl;

        }
        else if (pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 0) {
            topFaceCounterClockwise = true;
            std::cout << "TopfaceCounterClockwise Clicked" << std::endl;

        }
        else if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0) {
            rightFaceCounterClockwise = true;
            std::cout << "RightfaceCounterClockwise Clicked" << std::endl;

        }
        //glutSwapBuffers();

        glutPostRedisplay();

    }

    else if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(vao[8]);
        model_view[8] = (Translate(vec3(-spacingBetweenCubes / 2 + 0.1, -spacingBetweenCubes / 2, 0.8)) * Scale(2.0, 2.0, 1.0) *
            RotateX(Theta[Xaxis]) *
            RotateY(Theta[Yaxis]) *
            RotateZ(Theta[Zaxis]));
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view[8]);
        glDrawArrays(GL_TRIANGLES, 0, NumVertices);
        glFlush();


        y = glutGet(GLUT_WINDOW_HEIGHT) - y;
        unsigned char pixel[4];
        glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

        if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 255) {
            leftFaceCounterClockwise = true;
            std::cout << "LeftfaceCounterClockwise Clicked" << std::endl;

        }
        else if (pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 0) {
            topFaceCounterClockwise = true;
            std::cout << "TopfaceCounterClockwise Clicked" << std::endl;

        }
        else if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0) {
            rightFaceCounterClockwise = true;
            std::cout << "RightfaceCounterClockwise Clicked" << std::endl;

        }
        //glutSwapBuffers();

        glutPostRedisplay();
    }
}


//----------------------------------------------------------------------------
void timer(int p)
{

    /*Theta[Axis] += 1.0;
    if (Theta[Axis] > 360.0) {
        Theta[Axis] -= 360.0;
    }*/
    //glutPostRedisplay();

    glutTimerFunc(20.0, timer, 0);
}


//----------------------------------------------------------------------------

int
main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Color Cube");

    glewExperimental = GL_TRUE;
    glewInit();

    init();

    glutDisplayFunc(display); // set display callback function
    glutReshapeFunc(reshape);
    //glutIdleFunc( idle );//can also use idle event for animation instaed of timer
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    glutTimerFunc(2, timer, 0);

    glutMainLoop();
    return 0;
}