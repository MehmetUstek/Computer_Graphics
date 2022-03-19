//
//  Display a rotating cube, revisited with triangle list, timer callback and reshape
//

#include "Angel.h"
#include "objectType.cpp"

bool isSolid = true;
ObjectType object_type = ObjectType::CUBE;



typedef vec4  color4;
typedef vec4  point4;
color4 color;


const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 points[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA colors
color4 colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int  Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a vertex buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
    
    
    GLuint cube_indices[] = {
            0, 1, 2,
            2, 3, 0,
            1, 5, 6,
            6, 2, 1,
            7, 6, 5,
            5, 4, 7,
            4, 0, 3,
            3, 7, 4,
            4, 5, 1,
            1, 0, 4,
            3, 2, 6,
            6, 7, 3
        };
    
    // Create and initialize an index buffer object
    GLuint index_buffer;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
    
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
    
    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    
    mat4  projection;
    projection = Ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0); // Ortho(): user-defined function in mat.h
    //projection = Perspective( 45.0, 1.0, 0.5, 3.0 ); //try also perspective projection instead of ortho
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
    
    // Set current program object
    glUseProgram( program );
    
    // Enable hiddden surface removal
    glEnable( GL_DEPTH_TEST );
    
    // Set state variable "clear color" to clear buffer with.
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
   }

//----------------------------------------------------------------------------

void
display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //  Generate tha model-view matrix
    // Initial translation for putting into the view.
    const vec3 displacement( 0.0, 0.0, 0.0 );
    mat4 model_view = ( Translate( displacement ) * Scale(1.0, 1.0, 1.0) *
             RotateX( Theta[Xaxis] ) *
             RotateY( Theta[Yaxis] ) *
             RotateZ( Theta[Zaxis] ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
    
    // Solid
    if (isSolid) {
        glDrawElements(GL_TRIANGLES, NumVertices, GL_UNSIGNED_INT, 0);

    }
    // Wireframe
    else {
        glDrawElements(GL_LINE_LOOP, NumVertices, GL_UNSIGNED_INT, 0);
    }
    
    
    glutSwapBuffers();
    
}

//---------------------------------------------------------------------
//
// reshape
//

void reshape( int w, int h )
{
    glViewport( 0, 0, w, h );
    
    // Set projection matrix
    mat4  projection;
    if (w <= h)
        projection = Ortho(-1.0, 1.0, -1.0 * (GLfloat) h / (GLfloat) w,
                           1.0 * (GLfloat) h / (GLfloat) w, -1.0, 1.0);
    else  projection = Ortho(-1.0* (GLfloat) w / (GLfloat) h, 1.0 *
                             (GLfloat) w / (GLfloat) h, -1.0, 1.0, -1.0, 1.0);
    
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
    
    //reshape callback needs to be changed if perspective prohection is used
}


//----------------------------------------------------------------------------

void
idle( void )
{
    Theta[Axis] += 10.0;

    if ( Theta[Axis] > 360.0 ) {
        Theta[Axis] -= 360.0;
    }
    
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key,int x, int y )
{
    switch (key) {
    case 'q': case 'Q':
        exit(EXIT_SUCCESS);
        break;
    case 'd': case 'D':
        isSolid = !isSolid; // Change the wireframe or solid by pressing to d.
        break;
     // Colors ranging from black to white between the inputs from 1 to 8.
    case '1':
        color = color4(0.0, 0.0, 0.0, 1.0);  // black
        break;
    case '2':
        color = color4(1.0, 0.0, 0.0, 1.0);  // red
        break;
    case '3':
        color = color4(1.0, 1.0, 0.0, 1.0);  // yellow
        break;
    case '4':
        color = color4(0.0, 1.0, 0.0, 1.0);  // green
        break;
    case '5':
        color = color4(0.0, 0.0, 1.0, 1.0);  // blue
        break;
    case '6':
        color = color4(1.0, 0.0, 1.0, 1.0);  // magenta
        break;
    case '7':
        color = color4(1.0, 1.0, 1.0, 1.0);  // white
        break;
    case '8':
        color = color4(0.0, 1.0, 1.0, 1.0);   // cyan
        break;
    }
    


    
}

//----------------------------------------------------------------------------

void mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
        switch( button ) {
        case GLUT_LEFT_BUTTON:
            int temp_index = static_cast<int>(object_type) + 1;
            if (temp_index == 3) {
                temp_index = 0; // Cycle back.
            }
            object_type = static_cast<ObjectType>(temp_index);
            break;
        }
    }
}

//----------------------------------------------------------------------------
void timer( int p )
{
    Theta[Axis] += 1.0;
    if ( Theta[Axis] > 360.0 ) {
        Theta[Axis] -= 360.0;
    }
    const vec3 displacement(1, 0.0, 0.0);
    mat4 model_view = (Translate(displacement) * Scale(1.0, 1.0, 1.0) *
        RotateX(Theta[Xaxis]) *
        RotateY(Theta[Yaxis]) *
        RotateZ(Theta[Zaxis]));
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

    glutPostRedisplay();
    
    glutTimerFunc(20,timer,0);
}


//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode(  GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize( 512, 512 );
    glutInitWindowPosition( 50, 50 );
    glutCreateWindow( "Bouncing Objects" );
    
    glewExperimental = GL_TRUE;
    glewInit();

    init();
    
    glutDisplayFunc( display ); // set display callback function
    glutReshapeFunc( reshape );
    //glutIdleFunc( idle );//can also use idle event for animation instaed of timer
    glutMouseFunc( mouse );
    glutKeyboardFunc(keyboard);
    
    glutTimerFunc(2,timer,0);
    
    glutMainLoop();
    return 0;
}

