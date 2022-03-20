//
//  Display a rotating cube, revisited with triangle list, timer callback and reshape
//

#include "Angel.h"
#include "objectType.cpp"
#include "movement.cpp"

bool isSolid = true;
ObjectType object_type = ObjectType::CUBE;
// Window sizes:
GLsizei width = 760;
GLsizei height = 760;
ObjectLocation objectLocation;
GLuint buffer;



typedef vec4  color4;
typedef vec4  point4;
color4 color = color4(0.0, 0.0, 0.0, 1.0);  // black
GLuint vao[3];

//Sphere
const int NumTimesToSubdivide = 5;
const int NumTriangles = 4096;
// (4 faces)^(NumTimesToSubdivide + 1)
const int NumVertices_sphere = 3 * NumTriangles;
typedef Angel::vec4 point4;
typedef Angel::vec4 color4;
point4 points_sphere[NumVertices_sphere];
color4 colors_sphere[NumVertices_sphere];
int Index_sphere = 0;

void
triangle_sphere(const point4& a, const point4& b, const point4& c)
{
    colors_sphere[Index_sphere] = color; points_sphere[Index_sphere] = a; Index_sphere++;
    colors_sphere[Index_sphere] = color; points_sphere[Index_sphere] = b; Index_sphere++;
    colors_sphere[Index_sphere] = color; points_sphere[Index_sphere] = c; Index_sphere++;
}

point4
unit_sphere(const point4& p)
{
    float len = p.x * p.x + p.y * p.y + p.z * p.z;
    point4 t;
    if (len > DivideByZeroTolerance) {
        t = p / sqrt(len);
        t.w = 1.0;
    }
    return t;
}

void
divide_triangle_sphere(const point4& a, const point4& b,
    const point4& c, int count)
{
    if (count > 0) {
        point4 v1 = unit_sphere(a + b);
        point4 v2 = unit_sphere(a + c);
        point4 v3 = unit_sphere(b + c);
        divide_triangle_sphere(a, v1, v2, count - 1);
        divide_triangle_sphere(c, v2, v3, count - 1);
        divide_triangle_sphere(b, v3, v1, count - 1);
        divide_triangle_sphere(v1, v3, v2, count - 1);
    }
    else {
        triangle_sphere(a, b, c);
    }
}

void
tetrahedron_sphere(int count)
{
    point4 v[4] = {
    vec4(0.0, 0.0, 1.0, 1.0),
    vec4(0.0, 0.942809, -0.333333, 1.0),
    vec4(-0.816497, -0.471405, -0.333333, 1.0),
    vec4(0.816497, -0.471405, -0.333333, 1.0)
    };
    divide_triangle_sphere(v[0], v[1], v[2], count);
    divide_triangle_sphere(v[3], v[2], v[1], count);
    divide_triangle_sphere(v[0], v[3], v[1], count);
    divide_triangle_sphere(v[0], v[2], v[3], count);
}





const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 points[8] = {
    point4( -0.2, -0.2,  0.2, 1.0 ),
    point4( -0.2,  0.2,  0.2, 1.0 ),
    point4(  0.2,  0.2,  0.2, 1.0 ),
    point4(  0.2, -0.2,  0.2, 1.0 ),
    point4( -0.2, -0.2, -0.2, 1.0 ),
    point4( -0.2,  0.2, -0.2, 1.0 ),
    point4(  0.2,  0.2, -0.2, 1.0 ),
    point4(  0.2, -0.2, -0.2, 1.0 )
};

// RGBA colors
color4 colors[8] = {
    color, 
    color, 
    color, 
    color, 
    color, 
    color, 
    color, 
    color, 
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
    tetrahedron_sphere(NumTimesToSubdivide);
    
    objectLocation.initObjectLocation(-1.0 + 0.3, 0.5, 0.001, -0.001);

    glGenVertexArrays( 3, vao );
    glBindVertexArray( vao[0] );
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");


    // Create and initialize a vertex buffer object
    
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
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
    
    // Sphere
    glBindVertexArray(vao[1]);
    GLuint buffer_sphere;
    glGenBuffers(1, &buffer_sphere);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_sphere);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(colors_sphere),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points_sphere), points_sphere);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_sphere),
        sizeof(colors_sphere), colors_sphere);

    /*GLuint index_buffer_sphere;
    glGenBuffers(1, &index_buffer_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_sphere);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);*/

    
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor_sphere = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor_sphere);
    glVertexAttribPointer(vColor_sphere, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_sphere)));


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
    glClearColor( 0.0, 1.0, 1.0, 1.0 );
   }

//----------------------------------------------------------------------------

void
display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //  Generate tha model-view matrix
    // Initial translation for putting into the view.
    const vec3 displacement( objectLocation.locX, objectLocation.locY, 0.0 );
    mat4 model_view = ( Translate( displacement ) * Scale(1.0, 1.0, 1.0) *
             RotateX( Theta[Xaxis] ) *
             RotateY( Theta[Yaxis] ) *
             RotateZ( Theta[Zaxis] ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
    
    // Solid
    
    switch (object_type) {

    case ObjectType::CUBE:
        for (int i = 0; i < 8; i++) {
            colors[i] = color;
        }
        glBindVertexArray(vao[0]);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
        if (isSolid) {
            glDrawElements(GL_TRIANGLES, NumVertices, GL_UNSIGNED_INT, 0);
        }
        // Wireframe
        else {
            glDrawElements(GL_LINE_LOOP, NumVertices, GL_UNSIGNED_INT, 0);
        }
        break;
    case ObjectType::SPHERE:
        for (int i = 0; i < NumVertices_sphere; i++) {
            colors_sphere[i] = color;
        }
        glBindVertexArray(vao[1]);
        GLuint buffer_sphere;
        glGenBuffers(1, &buffer_sphere);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_sphere);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(colors_sphere),
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points_sphere), points_sphere);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_sphere),
            sizeof(colors_sphere), colors_sphere);
        if (isSolid) {
            glDrawArrays(GL_TRIANGLES, 0, NumVertices_sphere);
        }
        // Wireframe
        else {
            glDrawArrays(GL_LINE_LOOP, 0, NumVertices_sphere);
        }
        break;
        
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
    Theta[Axis] += 0.2;

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
    case 'i': case 'I':
        objectLocation.initObjectLocation(-1.0 + 0.3, 0.5, 0.001, -0.001);
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
    /*Theta[Axis] += 1.0;
    if ( Theta[Axis] > 360.0 ) {
        Theta[Axis] -= 360.0;
    }*/
    objectLocation.updateObjectLocation(0.2,0.2);
    /*const vec3 displacement(1, 0.0, 0.0);
    mat4 model_view = (Translate(displacement) * Scale(1.0, 1.0, 1.0) *
        RotateX(Theta[Xaxis]) *
        RotateY(Theta[Yaxis]) *
        RotateZ(Theta[Zaxis]));*/

    //glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

    glutPostRedisplay();
    
    glutTimerFunc(2,timer,0);
}


//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode(  GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize( width, height );
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

