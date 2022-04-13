//
//  Display a rotating cube, revisited
//

#include "Angel.h"

typedef vec4  color4;
typedef vec4  point4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
const int NumSquares = 8;
const float scaleConst = 2.0f;
const float squareWidth = 0.5f;
const float spacingBetweenCubes = squareWidth * 2+ 0.05;

point4 points[NumSquares][NumVertices];
color4 colors[NumSquares][NumVertices];
const int xoff = 1, yoff = 4, zoff = 16;

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
    color4(1.0, 0.0, 1.0, 1.0),  // magenta
    color4(1.0, 0.0, 0.0, 1.0),  // red
    color4(1.0, 1.0, 0.0, 1.0),  // yellow
    color4(0.0, 0.0, 1.0, 1.0),  // blue
    color4(0.0, 1.0, 0.0, 1.0),  // green
    
    color4(1.0, 0.5, 0.0, 1.0),  // orange
};

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int  Axis = Zaxis;
GLfloat  Theta[NumAxes] = { 30.0, -45.0, 0.0 };

// Model-view and projection matrices uniform location
GLuint ModelView, Projection;
GLuint Color;

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices

int Index = 0;
int colorIndex = 0;

void
quad(int a, int b, int c, int d, bool isFaceVisible, int cubeIndex)
{
    color4 faceColor;
    // Initialize colors
    if (!isFaceVisible) {
        faceColor = color4(0.0, 0.0, 0.0, 1.0);  // black
    }
    else {
        faceColor = color_cycle[colorIndex];
    }
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
    quad(1, 0, 3, 2, true, cubeIndex);
    quad(2, 3, 7, 6, true, cubeIndex);
    quad(3, 0, 4, 7, true, cubeIndex);
    quad(3, 0, 4, 7, true, cubeIndex);
    quad(6, 5, 1, 2, true, cubeIndex);
    quad(4, 5, 6, 7, true, cubeIndex);
    quad(5, 4, 0, 1, true, cubeIndex);
    Index = 0;
    colorIndex = 0;
}

//----------------------------------------------------------------------------

// OpenGL initialization
bool topFaceClockwise = false; bool topFaceCounterClockwise = false;
bool rightFaceClockwise = false; bool rightFaceCounterClockwise = false;
bool frontFaceClockwise = false; bool frontFaceCounterClockwise = false;
bool turn_up = false; bool turn_down = false; bool turn_right = false; bool turn_left = false;

GLuint vao[NumSquares];
GLuint vPosition, vColor;
GLuint buffer;
mat4 model_view[NumSquares];
void
init()
{
    glGenVertexArrays(NumSquares, vao);
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    for (int i = 0; i < NumSquares; i++) {
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
        //  Generate tha model-view matrix
        vec3 displacement;
        if (i == 0) {
            displacement = vec3(startingX, startingY, startingZ);
        }
        else if (i % 2 ==0) {
            if (i % 4 == 0) {
                startingZ = startingZ - spacingBetweenCubes;
                updatedY = startingY -spacingBetweenCubes * float(Theta[Yaxis]/140);
                startingX = startingX + float(Theta[Xaxis] / 90)*2;
                displacement = vec3(startingX, updatedY, startingZ);

            }
            else {
                updatedY = updatedY + spacingBetweenCubes * -float(Theta[Yaxis] / 50);
                startingZ += 0.4;
                displacement = vec3(startingX, updatedY, startingZ);
            }
            
        }
        else {
            // 1,3,5,7 The cubes at right at creation.
            displacement = vec3(startingX + (spacingBetweenCubes*float(Theta[Xaxis] / 45)), updatedY + spacingBetweenCubes *  float(Theta[Yaxis] / 135), startingZ+0.6);
        }
        
        mat4 model_view = (Translate(displacement) * Scale(1.0, 1.0, 1.0) *
            RotateX(Theta[Xaxis]) *
            RotateY(Theta[Yaxis]) *
            RotateZ(Theta[Zaxis]));

        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

        glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    }

    glutSwapBuffers();

}

//---------------------------------------------------------------------
//
// reshape
//

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


//----------------------------------------------------------------------------

void
idle(void)
{
    Theta[Axis] += 10.0;

    if (Theta[Axis] > 360.0) {
        Theta[Axis] -= 360.0;
    }

    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
    if (key == 'Q' | key == 'q') exit(0);
    if (key == 'w' | key == 'W') turn_up = true;
    if (key == 's' | key == 'S') turn_down = true;
    if (key == 'a' | key == 'A') turn_left = true;
    if (key == 'd' | key == 'D') turn_right = true;


}

//----------------------------------------------------------------------------

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(vao[8]);
        model_view[8] = (RotateX(45.0) * RotateY(-45.0) * RotateZ(0.0));
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view[8]);
        glDrawArrays(GL_TRIANGLES, 0, 288);
        glFlush();

        y = glutGet(GLUT_WINDOW_HEIGHT) - y;
        unsigned char pixel[4];
        glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

        if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 255) {
            topFaceClockwise = true; 
            std::cout << "TopfaceClockwise Clicked" << std::endl;
        }
        if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0) {
            frontFaceClockwise = true;
            std::cout << "FrontfaceClockwise Clicked" << std::endl;
        }
        if (pixel[0] == 255 && pixel[1] == 255 && pixel[2] == 255) {
            rightFaceClockwise = true;
            std::cout << "RightfaceClockwise Clicked" << std::endl;
        }

        glutPostRedisplay();

    }

    else if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(vao[8]);
        model_view[8] = (RotateX(45.0) * RotateY(-45.0) * RotateZ(0.0));
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view[8]);
        glDrawArrays(GL_TRIANGLES, 0, 288);
        glFlush();

        unsigned char pixel[4];
        y = glutGet(GLUT_WINDOW_HEIGHT) - y;
        glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

        if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 255) topFaceCounterClockwise = true;
        if (pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0) frontFaceCounterClockwise = true;
        if (pixel[0] == 255 && pixel[1] == 255 && pixel[2] == 255) rightFaceCounterClockwise = true;

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
    glutPostRedisplay();

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
