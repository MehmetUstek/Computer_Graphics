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
const float spacingBetweenCubes = squareWidth * 2;

point4 points[NumSquares][NumVertices];
color4 colors[NumSquares][NumVertices];

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
    color4(1.0, 0.5, 0.0, 1.0),  // orange
    color4(1.0, 0.0, 0.0, 1.0),  // red
    color4(1.0, 1.0, 0.0, 1.0),  // yellow
    color4(0.0, 0.0, 1.0, 1.0),  // blue
    color4(0.0, 1.0, 0.0, 1.0),  // green
    color4(1.0, 0.0, 1.0, 1.0),  // magenta
};

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int  Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 135.0, 0.0, 45.0 };

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;

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
    if (cubeIndex != 0) {
        point4 temp = point4(0, 0, 0.0, 1.0);
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[a] + temp; Index++;
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[b] + temp; Index++;
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[c] + temp; Index++;
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[a] + temp; Index++;
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[c] + temp; Index++;
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[d] + temp; Index++;
        colorIndex++;
    }
    else {
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[a]; Index++;
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[b]; Index++;
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[c]; Index++;
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[a]; Index++;
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[c]; Index++;
        colors[cubeIndex][Index] = faceColor; points[cubeIndex][Index] = vertices[d]; Index++;
        colorIndex++;
    }
}


//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube(bool isMiddleObject, int cubeIndex)
{
    quad(1, 0, 3, 2, true, cubeIndex);
    quad(2, 3, 7, 6, true, cubeIndex);
    if (isMiddleObject) {
        quad(3, 0, 4, 7, false, cubeIndex);
    }
    else {
        quad(3, 0, 4, 7, true, cubeIndex);
    }
    quad(6, 5, 1, 2, false, cubeIndex);
    quad(4, 5, 6, 7, false, cubeIndex);
    quad(5, 4, 0, 1, false, cubeIndex);
    colorIndex = 0;
}

//----------------------------------------------------------------------------

// OpenGL initialization

GLuint vao[2];
void
init()
{
    colorcube(false,0);
    colorcube(false,1);

    // Create a vertex array object
    
    glGenVertexArrays(2, vao);
    glBindVertexArray(vao[0]);

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points[0]) + sizeof(colors[0]), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points[0]), points[0]);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points[0]), sizeof(colors[0]), colors[0]);

    // Load shaders and use the resulting shader program
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");

    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points[0])));




    //////////////
    glBindVertexArray(vao[1]);

    // Create and initialize a buffer object
    GLuint buffer2;
    glGenBuffers(1, &buffer2);
    glBindBuffer(GL_ARRAY_BUFFER, buffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points[1]) + sizeof(colors[1]), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points[1]), points[1]);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points[1]), sizeof(colors[1]), colors[1]);


    // set up vertex arrays
    GLuint vPosition1 = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor1 = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points[1])));



    ///////////


    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation(program, "ModelView");
    Projection = glGetUniformLocation(program, "Projection");

    mat4  projection;
    projection = Ortho(-scaleConst, scaleConst, -scaleConst, scaleConst, -scaleConst, scaleConst); // Ortho(): user-defined function in mat.h
    //projection = Perspective( 90.0, 3.0, 15.0, 3.0 ); //try also perspective projection instead of ortho
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

    // Set current program object
    glUseProgram(program);

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
    glBindVertexArray(vao[0]);
    //  Generate tha model-view matrix
    const vec3 displacement(0.0, 0.0, 0.0);
    mat4 model_view = (Translate(displacement) * Scale(1.0, 1.0, 1.0) *
        RotateX(Theta[Xaxis]) *
        RotateY(Theta[Yaxis]) *
        RotateZ(Theta[Zaxis]));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

    glBindVertexArray(vao[1]);
    //  Generate tha model-view matrix
    const vec3 displacement2(0.0, 0.0, 0.0);
    mat4 model_view2 = (Translate(displacement2) * Scale(1.0, 1.0, 1.0) *
        RotateX(Theta[Xaxis]) *
        RotateY(Theta[Yaxis]) *
        RotateZ(Theta[Zaxis]));

    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view2);

    glDrawArrays(GL_TRIANGLES, 0, NumVertices);

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
    if (key == 'Q' | key == 'q')
        exit(0);

}

//----------------------------------------------------------------------------

void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        switch (button) {
        case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
        case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
        case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
        }
    }
}

//----------------------------------------------------------------------------
void timer(int p)
{
    
    Theta[Axis] += 1.0;
    if (Theta[Axis] > 360.0) {
        Theta[Axis] -= 360.0;
    }
    glutPostRedisplay();

    glutTimerFunc(20.0, timer, 0);
}


//----------------------------------------------------------------------------

int
main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
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
