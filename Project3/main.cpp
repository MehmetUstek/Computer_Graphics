#include "Angel.h"
#include "objectType.cpp"
#include "movement.cpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include "DrawingType.cpp"

// The drawing mode controller
bool isSolid = true;
// Initial object -> cube, then with mouse click, sphere, and then bunny.
ObjectType object_type = ObjectType::CUBE;
DrawingType drawing_type = DrawingType::WIREFRAME;
// Window sizes:
GLsizei width = 760;
GLsizei height = 760;
ObjectLocation objectLocation;
GLuint buffer;
GLuint buffer_sphere;
GLuint buffer_bunny;
float projection_constant = 5.0f;
const float velocityConst = 0.01;


typedef vec4  color4;
typedef vec4  point4;
color4 color = color4(0.0, 0.0, 0.0, 1.0);  // black
GLuint vao[3];


using std::cout; using std::cerr;
using std::endl; using std::string;
using std::ifstream; using std::vector;



const int numVertices_bunny = 4922;
const int numTriangles_bunny = 9840;
color4 colors_bunny[numVertices_bunny];
point4 vertex_list_bunny[numVertices_bunny];
int triangle_list_bunny[numTriangles_bunny*3];


// Read bunny.off, extract vertices, and triangles list. Since we know vertices and triangles size, to avoid dynamic array approach, I
// specified the numVertices and numTriangles beforehand.
void
readBunny() {
    std::string input;
    int numOfVertices, numOfTriangles;
    int dumm;

    std::ifstream file("bunny.off");
    file >> input;
    file >> numOfVertices >> numOfTriangles >> dumm;


    for (int i = 0; i < numOfVertices; i++) {
        GLfloat x, y, z;
        file >> x >> y >> z;
        vertex_list_bunny[i] = point4(x, y, z, (GLfloat)1.0);
    }

    for (int i = 0; i < numOfTriangles; i++) {
        int dummy;
        int x, y, z;
        file >> dummy >> x >> y >> z;
        triangle_list_bunny[3 * i] = x;
        triangle_list_bunny[(3 * i)+1] = y;
        triangle_list_bunny[(3*i)+2] = z;
    }
    file.close();
}


//Sphere 
// These codes along with the triangle_sphere, unit_sphere, divide_triangle_sphere, and tetrahedron_sphere functions
// are retrieved from the course textbook, appendix 7.
const int NumTimesToSubdivide = 5;
const int NumTriangles = 4096;
// (4 faces)^(NumTimesToSubdivide + 1)
const int NumVertices_sphere = 3 * NumTriangles;
typedef Angel::vec4 point4;
typedef Angel::vec4 color4;
point4 points_sphere[NumVertices_sphere];
color4 colors_sphere[NumVertices_sphere];
int Index_sphere = 0;
GLuint sphere_indices[NumVertices_sphere];
float scale = 1.0f;
GLfloat widthRatio = 1.0;
GLfloat heightRatio = 1.0;


// The normals are deleted, instead I added sphere_indices to get the sphere index values from each vertex to another.
void
triangle_sphere(const point4& a, const point4& b, const point4& c)
{
    sphere_indices[Index_sphere] = Index_sphere; colors_sphere[Index_sphere] = color; points_sphere[Index_sphere] = a; Index_sphere++;
    sphere_indices[Index_sphere] = Index_sphere; colors_sphere[Index_sphere] = color; points_sphere[Index_sphere] = b; Index_sphere++;
    sphere_indices[Index_sphere] = Index_sphere; colors_sphere[Index_sphere] = color; points_sphere[Index_sphere] = c; Index_sphere++;
}

point4
unit_sphere(const point4& p)
{
    float len = (p.x * p.x + p.y * p.y + p.z * p.z);
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
    point4(-0.5, -0.5,  0.5, 1.0),
    point4(-0.5,  0.5,  0.5, 1.0),
    point4(0.5,  0.5,  0.5, 1.0),
    point4(0.5, -0.5,  0.5, 1.0),
    point4(-0.5, -0.5, -0.5, 1.0),
    point4(-0.5,  0.5, -0.5, 1.0),
    point4(0.5,  0.5, -0.5, 1.0),
    point4(0.5, -0.5, -0.5, 1.0)
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



// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
    cout << "-h: Help" << endl << "-q: Quit program" << endl << "-i: Initilization of the animation, starting from the top left corner" << endl;
    cout << "-d: Change drawing mode between solid and wireframe" << endl;
    cout << "Change object type with mouse Events:" << endl << " Left click for changing between the objects, starting from cube, then sphere and lastly the bunny" << endl << " The objects will circle back to cube after bunny" << endl;
    cout << "Change color with numpad actions from 1-8:" << endl;
    cout << "  1: Black" << endl << "  2: Red" << endl << "  3: Yellow" << endl << "  4: Green" << endl << "  5: Blue" << endl << "  6: Magenta" << endl << "  7: White" << endl << "  8: Cyan" << endl << endl << endl;
    
    readBunny(); // read bunny and extract triangles and vertex list, which will then used for bunny creation.
    tetrahedron_sphere(NumTimesToSubdivide); // Create sphere.
    for (int i = 0; i < numVertices_bunny; i++) {
        colors_bunny[i] = color; // Assign bunny colors at initialization. (to black)
    }
    
    objectLocation.initObjectLocation(-projection_constant + scale + 0.2, scale+0.3, velocityConst, -2*velocityConst, projection_constant);

    glGenVertexArrays( 3, vao );
    glBindVertexArray( vao[0] );
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    
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
    
    // Create and initialize an index buffer object for cube.
    GLuint index_buffer;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
    
    // Sphere object binding and creation.
    glBindVertexArray(vao[1]);
    glGenBuffers(1, &buffer_sphere);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_sphere);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(colors_sphere),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points_sphere), points_sphere);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_sphere),
        sizeof(colors_sphere), colors_sphere);

    // Create and initialize an index buffer object for sphere.
    GLuint index_buffer_sphere;
    glGenBuffers(1, &index_buffer_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_sphere);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_indices), sphere_indices, GL_STATIC_DRAW);

    GLuint vPosition1 = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition1);
    glVertexAttribPointer(vPosition1, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor_sphere = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor_sphere);
    glVertexAttribPointer(vColor_sphere, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_sphere)));


    // Bunny object binding and creation.
    glBindVertexArray(vao[2]);
    glGenBuffers(1, &buffer_bunny);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_bunny);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list_bunny) + sizeof(colors_bunny),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_list_bunny), vertex_list_bunny);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex_list_bunny),
        sizeof(colors_bunny), colors_bunny);

    // Create and initialize an index buffer object for bunny.
    GLuint index_buffer_bunny;
    glGenBuffers(1, &index_buffer_bunny);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_bunny);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangle_list_bunny), triangle_list_bunny, GL_STATIC_DRAW);

    GLuint vPosition2 = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition2);
    glVertexAttribPointer(vPosition2, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor_bunny = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor_bunny);
    glVertexAttribPointer(vColor_bunny, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertex_list_bunny)));



    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    
    
    mat4  projection;
    projection = Ortho(-projection_constant *widthRatio, projection_constant * widthRatio, -projection_constant * heightRatio, projection_constant * heightRatio, -projection_constant, projection_constant); // Ortho(): user-defined function in mat.h
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
    const vec3 displacement(objectLocation.locX, objectLocation.locY, 0.0);
    mat4 model_view;


    switch (object_type) {

    case ObjectType::CUBE:
        for (int i = 0; i < 8; i++) {
            colors[i] = color; // Reassign colors for cube. And then rebind the points and color data to reflect the color changes.
        }
        glBindVertexArray(vao[0]);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
        switch (drawing_type)
        {
        case DrawingType::WIREFRAME:
            glDrawElements(GL_LINE_LOOP, NumVertices, GL_UNSIGNED_INT, 0);
            break;
        case DrawingType::SOLID:
            glDrawElements(GL_TRIANGLES, NumVertices, GL_UNSIGNED_INT, 0);
            break;

        }
        model_view = (Translate(displacement) * Scale(scale, scale, scale));
        break;
    case ObjectType::SPHERE:

        for (int i = 0; i < NumVertices_sphere; i++) {
            colors_sphere[i] = color; // Reassign colors for sphere. And then rebind the points and color data to reflect the color changes.
        }
        glBindVertexArray(vao[1]);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_sphere);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(colors_sphere),
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points_sphere), points_sphere);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_sphere),
            sizeof(colors_sphere), colors_sphere);
        switch (drawing_type)
        {
        case DrawingType::WIREFRAME:
            glDrawElements(GL_LINE_LOOP, NumVertices_sphere, GL_UNSIGNED_INT, 0);
            break;
        case DrawingType::SOLID:
            glDrawElements(GL_TRIANGLES, NumVertices_sphere, GL_UNSIGNED_INT, 0);
            break;

        }
        //if (isSolid) {
        //    glDrawElements(GL_TRIANGLES, NumVertices_sphere, GL_UNSIGNED_INT, 0);
        //}
        //// Wireframe
        //else {
        //    glDrawElements(GL_LINE_LOOP, NumVertices_sphere, GL_UNSIGNED_INT, 0);
        //}
        
        model_view = (Translate(displacement) * Scale(scale, scale, scale));
        break;
    case ObjectType::BUNNY:
        for (int i = 0; i < numVertices_bunny; i++) {
            colors_bunny[i] = color; // Reassign colors for bunny. And then rebind the points and color data to reflect the color changes.
        }
        glBindVertexArray(vao[2]);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_bunny);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list_bunny) + sizeof(colors_bunny),
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_list_bunny), vertex_list_bunny);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex_list_bunny),
            sizeof(colors_bunny), colors_bunny);
        switch (drawing_type)
        {
        case DrawingType::WIREFRAME:
            glDrawElements(GL_LINE_LOOP, numTriangles_bunny*3, GL_UNSIGNED_INT, 0);
            break;
        case DrawingType::SOLID:
            glDrawElements(GL_TRIANGLES, numTriangles_bunny * 3, GL_UNSIGNED_INT, 0);
            break;

        }

        model_view = (Translate(displacement) * Scale(scale / 8, scale / 8, scale / 8));
        break;


    }
    
    mat4  projection;
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
    projection = Ortho(-5.0 * widthRatio, 5.0 * widthRatio, -5.0 * heightRatio, 5.0 * heightRatio, -5.0, 5.0); // Ortho(): user-defined function in mat.h

    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
    
    
    glutSwapBuffers();
    
}

//---------------------------------------------------------------------
//
// reshape
//

void reshape( int w, int h )
{
    glViewport( 0, 0, w, h );
    scale = (GLfloat)w / (GLfloat)h;
    widthRatio = (GLfloat)w / 760;
    heightRatio = (GLfloat)h / 760;
}



//----------------------------------------------------------------------------

void
keyboard( unsigned char key,int x, int y )
{
    switch (key) {
    case 'q': case 'Q':
        exit(EXIT_SUCCESS);
        break;
    case 'h': case 'H':
        cout << "-h: Help:" << endl << "-q: Quit program" << endl << "-i: Initilization of the animation, starting from the top left corner" << endl;
        cout << "Change object type with mouse Events:" << endl << " Left click for changing between the objects, starting from cube, then sphere and lastly the bunny" << endl << " The objects will circle back to cube after bunny" << endl;
        cout << "Change color with numpad actions from 1-8:" << endl;
        cout << "  1: Black" << endl << "  2: Red" << endl << "  3: Yellow" << endl << "  4: Green" << endl << "  5: Blue" << endl << "  6: Magenta" << endl << "  7: White" << endl << "  8: Cyan" << endl;
        break;
    case 'i': case 'I':
        objectLocation.initObjectLocation(-projection_constant + 2*scale, scale + 0.1, velocityConst, -2 * velocityConst, projection_constant);
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
    // Mouse click event: Change object type if the left mouse button is down.
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
    // Update the object location at each time passed. The radius are updated since there can be change in object types and objects do not have the same scale.
    // Thus prevents resulting in unwanted scale problems.
    switch (object_type) {

    case ObjectType::CUBE:
        objectLocation.updateObjectLocation(scale/2, scale/2, widthRatio, heightRatio);
        break;
    case ObjectType::SPHERE:
        objectLocation.updateObjectLocation(scale, scale, widthRatio, heightRatio);
        break;
    case ObjectType::BUNNY:
        objectLocation.updateObjectLocation(scale, scale*1.8, widthRatio, heightRatio);
        break;

    };

    glutPostRedisplay();
    
    glutTimerFunc(2,timer,0);
}

void menuStart(int id) {

}
void shadingMenu(int id) {

}
void componentMenu(int id) {

}
void materialProperty(int id) {

}
void lightSource(int id) {

}

void displayMode(int id) {
    switch (id) {
    case 1:
        drawing_type = DrawingType::WIREFRAME;
        break;
    case 2:
        drawing_type = DrawingType::SHADING;
        break;
    case 3:
        drawing_type = DrawingType::TEXTURE;
        break;
    case 4:
        drawing_type = DrawingType::SOLID;
        break;
    }
    glutPostRedisplay();
}

void glutMenu() {
    int shading_menu, shading_component, light_source, material_property, display_mode;
    shading_menu = glutCreateMenu(shadingMenu);
    glutAddMenuEntry("Gouraud", 1);
    glutAddMenuEntry("Phong", 2);
    glutAddMenuEntry("Modified Phong", 3);
    shading_component = glutCreateMenu(componentMenu);
    glutAddMenuEntry("Specular", 1);
    glutAddMenuEntry("Diffuse", 2);
    glutAddMenuEntry("Ambient", 3);
    

    material_property = glutCreateMenu(materialProperty);
    glutAddMenuEntry("Plastic", 1);
    glutAddMenuEntry("Metallic", 2);

    light_source = glutCreateMenu(lightSource);
    glutAddMenuEntry("Fixed", 1);
    glutAddMenuEntry("Move with object", 2);

    display_mode = glutCreateMenu(displayMode);
    glutAddMenuEntry("Wireframe", 1);
    glutAddMenuEntry("Shading", 2);
    glutAddMenuEntry("Texture", 3);
    glutAddMenuEntry("SOLID", 4);

    // Main Menu
    glutCreateMenu(menuStart);
    glutAddSubMenu("Shading", shading_menu);
    glutAddSubMenu("Shading Component", shading_component);
    glutAddSubMenu("Material", material_property);
    glutAddSubMenu("Light Source", light_source);
    glutAddSubMenu("Display Mode", display_mode);
    
    


    //glutCreateMenu(menuStart);
    //glutAddMenuEntry
    glutAttachMenu(GLUT_RIGHT_BUTTON);
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
    glutMenu();
    
    glutDisplayFunc( display ); // set display callback function
    glutReshapeFunc( reshape );
    glutMouseFunc( mouse );
    glutKeyboardFunc(keyboard);
    
    glutTimerFunc(2,timer,0);
    
    glutMainLoop();
    return 0;
}

