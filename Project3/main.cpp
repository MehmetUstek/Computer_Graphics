#include "Angel.h"
#include "objectType.cpp"
#include "movement.cpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include "DrawingType.cpp"
#include "ShadingMode.cpp"


enum {
    GOURAUD = 0,
    PHONG = 1,
    MODIFIED_PHONG = 2
};
// Initial object -> cube, then with mouse click, sphere, and then bunny.
ObjectType object_type = ObjectType::SPHERE;
DrawingType drawing_type = DrawingType::WIREFRAME;
int shading_mode = GOURAUD;
// Window sizes:
GLsizei width = 760;
GLsizei height = 760;
ObjectLocation objectLocation;
GLuint buffer_sphere;
float projection_constant = 5.0f;
const float velocityConst = 0.01;


typedef vec4  color4;
typedef vec4  point4;
color4 color = color4(0.0, 0.0, 0.0, 1.0);  // black
GLuint vao[1];


using std::cout; using std::cerr;
using std::endl; using std::string;
using std::ifstream; using std::vector;


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
int Index_sphere = 0;
GLuint sphere_indices[NumVertices_sphere];
float scale = 1.0f;
GLfloat widthRatio = 1.0;
GLfloat heightRatio = 1.0;
vec3 normals_sphere[NumVertices_sphere];


// The normals are deleted, instead I added sphere_indices to get the sphere index values from each vertex to another.
void
triangle_sphere(const point4& a, const point4& b, const point4& c)
{
    vec3  normal = normalize(cross(b - a, c - b));

    normals_sphere[Index_sphere] = normal; points_sphere[Index_sphere] = a; Index_sphere++;
    normals_sphere[Index_sphere] = normal; points_sphere[Index_sphere] = b; Index_sphere++;
    normals_sphere[Index_sphere] = normal; points_sphere[Index_sphere] = c; Index_sphere++;
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


// Model-view and projection matrices uniform location
GLuint  ModelView, Projection, vPosition, vNormal, vCoords, Shading_Mode, AmbientProduct, DiffuseProduct, SpecularProduct, Light1Position, Light2Position, Shininess;
GLuint LightToggle1, LightToggle2, customTexture;

//----------------------------------------------------------------------------
void setProjection(void) {
    mat4  projection;
    projection = Ortho(-projection_constant * widthRatio, projection_constant * widthRatio, -projection_constant * heightRatio, projection_constant * heightRatio, -projection_constant, projection_constant); // Ortho(): user-defined function in mat.h
    //projection = Perspective(45.0, scale, -projection_constant, projection_constant);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
}
// OpenGL initialization
void
init()
{
    tetrahedron_sphere(NumTimesToSubdivide); // Create sphere.
    
    objectLocation.initObjectLocation(-projection_constant + scale + 0.2, scale+0.3, velocityConst, -2*velocityConst, projection_constant);

    glGenVertexArrays( 1, vao );
    glBindVertexArray( vao[0] );
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    //GLuint program = InitShader("vshader_gouraud.glsl", "fshader_gouraud.glsl");
    
    // Sphere object binding and creation.
    glBindVertexArray(vao[1]);
    glGenBuffers(1, &buffer_sphere);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_sphere);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(normals_sphere), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points_sphere), points_sphere);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_sphere), sizeof(normals_sphere), normals_sphere);

    GLuint vPosition1 = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition1);
    glVertexAttribPointer(vPosition1, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint vColor_sphere = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor_sphere);
    glVertexAttribPointer(vColor_sphere, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_sphere)));


    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_sphere)));


    // Initialize shader lighting parameters
    point4 light_position(0.0, 0.0, 1.0, 0.0); //directional light source
    color4 light_ambient(0.2, 0.2, 0.2, 1.0);
    color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
    color4 light_specular(1.0, 1.0, 1.0, 1.0);

    color4 material_ambient(1.0, 0.0, 1.0, 1.0);
    color4 material_diffuse(1.0, 0.8, 0.0, 1.0);
    color4 material_specular(1.0, 0.8, 0.0, 1.0);
    float  material_shininess = 100.0;

    color4 ambient_product = light_ambient * material_ambient;
    color4 diffuse_product = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;

    glUniform4fv(glGetUniformLocation(program, "AmbientProduct"),
        1, ambient_product);
    glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"),
        1, diffuse_product);
    glUniform4fv(glGetUniformLocation(program, "SpecularProduct"),
        1, specular_product);

    glUniform4fv(glGetUniformLocation(program, "LightPosition"),
        1, light_position);

    glUniform1f(glGetUniformLocation(program, "Shininess"),
        material_shininess);


    vCoords = glGetAttribLocation(program, "vCoords");
    Shading_Mode = glGetUniformLocation(program, "Shading_Mode");

    glUniform1i(Shading_Mode, Shading_Mode);

    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    
    
    setProjection();
    
    // Set current program object
    glUseProgram( program );
    
    // Enable hiddden surface removal
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_CULL_FACE);
    
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

    case ObjectType::SPHERE:

        glBindVertexArray(vao[1]);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_sphere);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(normals_sphere),
            NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points_sphere), points_sphere);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_sphere),
            sizeof(normals_sphere), normals_sphere);
        switch (drawing_type)
        {
        case DrawingType::WIREFRAME:
            //glDrawElements(GL_LINE_LOOP, NumVertices_sphere, GL_UNSIGNED_INT, 0);
            glDrawArrays(GL_LINE_LOOP, 0, NumVertices_sphere);
            break;
        case DrawingType::SOLID:
            //glDrawElements(GL_TRIANGLES, NumVertices_sphere, GL_UNSIGNED_INT, 0);
            glDrawArrays(GL_TRIANGLES, 0, NumVertices_sphere);
            break;

        }
        
        //model_view = (Translate(displacement) * Scale(scale, scale, scale));
        break;

    }
    
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
    setProjection();
    
    
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
    case ObjectType::SPHERE:
        objectLocation.updateObjectLocation(scale, scale, widthRatio, heightRatio);
        break;

    };

    glutPostRedisplay();
    
    glutTimerFunc(2,timer,0);
}

void menuStart(int id) {

}
void shadingMenu(int id) {
    switch (id) {
    case 1:
        shading_mode = GOURAUD;
        break;
    case 2:
        shading_mode = PHONG;
        break;
    case 3:
        shading_mode = MODIFIED_PHONG;
        break;
    }
    //Shading_Mode = static_cast<int>(shading_mode);
    glUniform1i(Shading_Mode, shading_mode);
    glutPostRedisplay();
}
void componentMenu(int id) {
    switch (id) {
    case 1:
        break;
    }
    glutPostRedisplay();
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

