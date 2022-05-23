#include "Angel.h"
#include "objectType.cpp"
#include "movement.cpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include "DrawingType.cpp"
#include "ShadingMode.cpp"


enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
GLfloat Theta[NumAxes] = { 0.0, 0.0, 0.0 };

enum {
    GOURAUD = 0,
    PHONG = 1,
    MODIFIED_PHONG = 2
};
enum {
    WIREFRAME=0,
    SHADING=1,
    TEXTURE=2
};

enum {
    BASKETBALL = 0,
    EARTH = 1
};
// Initial object -> cube, then with mouse click, sphere, and then bunny.
ObjectType object_type = ObjectType::SPHERE;
int drawing_type = SHADING;
int shading_mode = GOURAUD;
int texture_type = BASKETBALL;
GLfloat u, v;
// Window sizes:
GLsizei width = 760;
GLsizei height = 760;
ObjectLocation objectLocation;
GLuint buffer_sphere;
float projection_constant = 1.0f;
const float velocityConst = 0.001;
bool lighting = true;
bool isFixed = false;
bool ambientFlag = true;
bool diffuseFlag = true;
bool specularFlag = true;

GLubyte* image;
GLubyte* image2;
GLubyte basketball[512][256][3];
const int  TextureSizeX = 512;
const int TextureSizeY = 256;

const int  TextureSizeX_2 = 2048;
const int TextureSizeY_2 = 1024;

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
color4  quad_colors[NumVertices_sphere];
vec2    tex_coords[NumVertices_sphere];
int Index_sphere = 0;
float scale = 1.0f;
GLfloat widthRatio = 1.0;
GLfloat heightRatio = 1.0;
vec3 normals_sphere[NumVertices_sphere];
GLuint textures[2];

// Read bunny.off, extract vertices, and triangles list. Since we know vertices and triangles size, to avoid dynamic array approach, I
// specified the numVertices and numTriangles beforehand.
//void
//readBunny() {
//    std::string input;
//    int numOfVertices, numOfTriangles;
//    int dumm;
//
//    std::ifstream file("bunny.off");
//    file >> input;
//    file >> numOfVertices >> numOfTriangles >> dumm;
//
//
//    for (int i = 0; i < numOfVertices; i++) {
//        GLfloat x, y, z;
//        file >> x >> y >> z;
//        vertex_list_bunny[i] = point4(x, y, z, (GLfloat)1.0);
//    }
//
//    for (int i = 0; i < numOfTriangles; i++) {
//        int dummy;
//        int x, y, z;
//        file >> dummy >> x >> y >> z;
//        triangle_list_bunny[3 * i] = x;
//        triangle_list_bunny[(3 * i) + 1] = y;
//        triangle_list_bunny[(3 * i) + 2] = z;
//    }
//    file.close();
//}

GLubyte*
readPPM(const char* filename) {
    int n, m;
    GLubyte* image;
    FILE* fd;
    int k, nm;
    char c;
    int i;
    char b[100];
    float s;
    int red, green, blue;
    fd = fopen(filename, "r");
    fscanf(fd, "%[^\n]", b);
    if (b[0] != 'P' || b[1] != '3') {
        printf("%s is not a PPM file!\n",b);
        exit(0);
    }
    printf("%s is a PPM file\n",b);
    fscanf(fd, "%c", &c);
    while (c == '#')
    {
        fscanf(fd, "%[^\n]",b);
        printf("%s\n", b);
        fscanf(fd, "%c", &c);
    }
    ungetc(c, fd);

    fscanf(fd, "%d %d %d", &n, &m, &k);
    printf("%d rows %d columns max value= %d\n", n, m, k);
    nm = n * m;
    image = (GLubyte*) malloc(3 * sizeof(GLuint) * nm);
    for (i = nm; i > 0; i--)
    {
        fscanf(fd, "%d %d %d", &red, &green, &blue);
        image[3 * nm - 3 * i] = red;
        image[3 * nm - 3 * i + 1] = green;
        image[3 * nm - 3 * i + 2] = blue;
    }
    return image;
    
}


void calculate_u_v(point4 point) {
    point4 V = normalize(point);
    double r = scale / 5;

    if (texture_type == BASKETBALL)
        r /= TextureSizeY;
    else if (texture_type == EARTH)
        r /= TextureSizeY_2;

    v = acos(V.z / r) / M_PI;
    if (V.y > 0)
        u = acos(V.x / (r * sin(M_PI * (v)))) / (2 * M_PI);
    else
        u = (M_PI + acos(V.x / (r * sin(M_PI * (v))))) / (2 * M_PI);
}

// The normals are deleted, instead I added sphere_indices to get the sphere index values from each vertex to another.
void
triangle_sphere(const point4& a, const point4& b, const point4& c)
{
    color4 colors[8] = {
    color4(0.0, 0.0, 0.0, 1.0),  // black
    color4(1.0, 0.0, 0.0, 1.0),  // red
    color4(1.0, 1.0, 0.0, 1.0),  // yellow
    color4(0.0, 1.0, 0.0, 1.0),  // green
    color4(0.0, 0.0, 1.0, 1.0),  // blue
    color4(1.0, 0.0, 1.0, 1.0),  // magenta
    color4(0.5, 0.5, 0.5, 1.0),  // white
    color4(1.0, 1.0, 1.0, 1.0)   // cyan
    };

    vec3  normal = normalize(cross(b - a, c - b));

    

    //TODO: add quads and textures.
    calculate_u_v(a); tex_coords[Index_sphere] = vec2(u, v); normals_sphere[Index_sphere] = normal; points_sphere[Index_sphere] = a; Index_sphere++;
    calculate_u_v(b); tex_coords[Index_sphere] = vec2(u, v); normals_sphere[Index_sphere] = normal; points_sphere[Index_sphere] = b; Index_sphere++;
    calculate_u_v(c); tex_coords[Index_sphere] = vec2(u, v); normals_sphere[Index_sphere] = normal; points_sphere[Index_sphere] = c; Index_sphere++;
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
GLuint  ModelView, Projection, vPosition, vNormal, vCoords, Shading_Mode, AmbientProduct, DiffuseProduct, SpecularProduct, Light1Position, Light2Position, Shininess, isLightSourceFixed;
GLuint customTexture, program, Drawing_Type;
bool textureFlag = true; //enable texture mapping
GLuint TextureFlagLoc; // texture flag uniform location
vec4 zero = vec4(0, 0, 0, 0);


// Initialize shader lighting parameters
point4 light_position(0.0, 0.0, 2.0, 1.0); //point light source.
color4 light_ambient(0.2, 0.2, 0.2, 1.0);
color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
color4 light_specular(1.0, 1.0, 1.0, 1.0);

color4 material_ambient(1.0, 0.0, 1.0, 1.0);
color4 material_diffuse(1.0, 0.8, 0.0, 1.0);
color4 material_specular(1.0, 0.8, 0.0, 1.0);
float material_shininess = 5.0;


color4 ambient_product = light_ambient * material_ambient;
color4 diffuse_product = light_diffuse * material_diffuse;
color4 specular_product = light_specular * material_specular;




//----------------------------------------------------------------------------
void setProjection(void) {
    mat4  projection;
    projection = Ortho(-projection_constant * widthRatio, projection_constant * widthRatio, -projection_constant * heightRatio, projection_constant * heightRatio, -projection_constant, projection_constant); // Ortho(): user-defined function in mat.h
    //projection = Perspective(45.0, scale, 0.1, 5.0);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
}
// OpenGL initialization
void
init()
{
    
    tetrahedron_sphere(NumTimesToSubdivide); // Create sphere.
    
    objectLocation.initObjectLocation((-projection_constant + scale + 0.2)/5, (scale+0.3)/5, velocityConst, -2*velocityConst);

    ///////////////////////
    // Texture
    // Initialize texture objects
    image = readPPM("basketball.ppm");
    image2 = readPPM("earth.ppm");
    glGenTextures(2, textures);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureSizeX, TextureSizeY, 0,
        GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //try here different alternatives
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //try here different alternatives

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureSizeX_2, TextureSizeY_2, 0,
        GL_RGB, GL_UNSIGNED_BYTE, image2);
    glGenerateMipmap(GL_TEXTURE_2D); // try also activating mipmaps for the second texture object
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]); //set current texture
    ////////////////////////
    /////////////////////




    glGenVertexArrays( 1, vao );
    glBindVertexArray( vao[0] );
    program = InitShader("vshader.glsl", "fshader.glsl");
    
    // Sphere object binding and creation.
    glBindVertexArray(vao[1]);
    glGenBuffers(1, &buffer_sphere);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_sphere);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(normals_sphere) + sizeof(quad_colors) + sizeof(tex_coords), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points_sphere), points_sphere);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_sphere), sizeof(normals_sphere), normals_sphere);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(normals_sphere), sizeof(quad_colors), quad_colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(normals_sphere)+ sizeof(quad_colors), sizeof(tex_coords), tex_coords);

    GLintptr offset = 0;
    offset = 0;
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));
    offset += sizeof(points_sphere);

    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));

    offset += sizeof(normals_sphere);

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));
    offset += sizeof(quad_colors);
    

    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));


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
    Drawing_Type = glGetUniformLocation(program, "Drawing_Type");
    isLightSourceFixed = glGetUniformLocation(program, "isLightSourceFixed");

    glUniform1i(Shading_Mode, shading_mode);
    glUniform1i(Drawing_Type, drawing_type);
    glUniform1i(isLightSourceFixed, isFixed);

    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );

    TextureFlagLoc = glGetUniformLocation(program, "TextureFlag");
    glUniform1i(TextureFlagLoc, textureFlag);
    
    
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

    point4 at(0.0, 0.0, 0.0, 1.0);
    point4 eye(0.0, 0.0, 2.0, 1.0);
    vec4 up(0.0, 1.0, 0.0, 0.0);
    mat4 model_view = LookAt(eye, at, up);
    const vec3 displacement(objectLocation.locX, objectLocation.locY, 0.0);
    //const vec3 displacement(0.0, 0.0, 0.0);


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
        case WIREFRAME:
            glDrawArrays(GL_LINE_LOOP, 0, NumVertices_sphere);
            break;
        case SHADING:
            if(lighting) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLES, 0, NumVertices_sphere);
            break;

        }
        
        model_view = (Translate(displacement) * Scale(scale/5, scale/5, scale/5)) * RotateZ(Theta[Zaxis]);
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
        objectLocation.initObjectLocation((-projection_constant + 2*scale)/5, (scale + 0.1)/5, velocityConst, -2 * velocityConst);
        break;
     // Colors ranging from black to white between the inputs from 1 to 8.
    case '1':
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        break;

    case '2':
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        break;
    case 't':
        if (textureFlag == true) textureFlag = false;
        else textureFlag = true;
        glUniform1i(TextureFlagLoc, textureFlag);
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
        objectLocation.updateObjectLocation(scale/5, scale/5, widthRatio, heightRatio);
        break;

    };
    Theta[Zaxis] += 3;

    if (Theta[Zaxis] > 360.0) {
        Theta[Zaxis] -= 360.0;
    }
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
    glUniform1i(Shading_Mode, shading_mode);
    glutPostRedisplay();
}
void componentMenu(int id) {
    switch (id) {
    case 1: // Specular
        if (specularFlag) {
            specular_product = zero;
            specularFlag = false;
        }
        else {
            specular_product = light_specular * material_specular;
            specularFlag = true;
        }
        break;
    case 2: // Diffuse
        if (diffuseFlag) {
            diffuse_product = zero;
            diffuseFlag = false;
        }
        else {
            diffuse_product = light_diffuse * material_diffuse;
            diffuseFlag = true;
        }
        break;
    case 3: // Ambient
        if (ambientFlag) {
            ambient_product = zero;
            ambientFlag = false;
        }
        else {
            ambient_product = light_ambient * material_ambient;
            ambientFlag = true;
        }
        break;
    }
    glUniform4fv(glGetUniformLocation(program, "AmbientProduct"),
        1, ambient_product);
    glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"),
        1, diffuse_product);
    glUniform4fv(glGetUniformLocation(program, "SpecularProduct"),
        1, specular_product);
    glutPostRedisplay();
}
void materialProperty(int id) {
    switch (id) {
    case 1:
        // Plastic 5-10
        material_shininess = 5.0;
        break;
    case 2:
        // Metallic 100-200
        material_shininess = 150.0;
        break;
    }
    glUniform1f(glGetUniformLocation(program, "Shininess"),
        material_shininess);
    glutPostRedisplay();
}
void lightSource(int id) {
    switch (id) {
    case 1:
        isFixed = false;
        break;
    case 2:
        isFixed = true;
        break;
    }
    glUniform1i(isLightSourceFixed, isFixed);
    glutPostRedisplay();

}

void displayMode(int id) {
    switch (id) {
    case 1:
        drawing_type = WIREFRAME;
        break;
    case 2:
        drawing_type = SHADING;
        break;
    case 3:
        drawing_type = TEXTURE;
        break;
    }
    glUniform1i(Drawing_Type, drawing_type);
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

