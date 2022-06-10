#include "Angel.h"
#include "objectType.cpp"
#include "movement.cpp"
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include "DrawingType.cpp"
#include "ShadingMode.cpp"

/// <summary>
/// @author Mehmet N. Ustek
/// Completeness of the project: All required cases are done. Bonus bunny part is also done.
/// </summary>

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

enum {
    PLASTIC = 5,
    METALLIC = 150
};
// Initial object -> cube, then with mouse click, sphere, and then bunny.
ObjectType object_type = ObjectType::SPHERE;
int drawing_type = SHADING;
int shading_mode = GOURAUD;
int texture_type = BASKETBALL;

// Window sizes:
GLsizei width = 760;
GLsizei height = 760;
ObjectLocation objectLocation;
GLuint buffer_sphere;
GLuint buffer_bunny;

float projection_constant = 1.0f;
const float velocityConst = 0.001;
bool lighting = true;
bool isFixed = false;
bool ambientFlag = true;
bool diffuseFlag = true;
bool specularFlag = true;

GLubyte* image;
GLubyte* image2;

const int  TextureSizeX = 512;
const int TextureSizeY = 256;
GLubyte basketball[TextureSizeX][TextureSizeY][3];

const int  TextureSizeX_2 = 2048;
const int TextureSizeY_2 = 1024;

GLubyte earth[TextureSizeX_2][TextureSizeY_2][3];

typedef vec4  color4;
typedef vec4  point4;
color4 color = color4(0.0, 0.0, 0.0, 1.0);  // black
GLuint vao[2];


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
color4 colors_sphere[NumVertices_sphere];
vec2    tex_coords[NumVertices_sphere];
int Index_sphere = 0;
float scale = 1.0f;
GLfloat widthRatio = 1.0;
GLfloat heightRatio = 1.0;
vec3 normals_sphere[NumVertices_sphere];
GLuint textures[2];

//Bunny
const int numVertices_bunny = 4922;
const int numTriangles_bunny = 9840;
color4 colors_bunny[numVertices_bunny];
color4 normals_bunny[numVertices_bunny];
point4 vertex_list_bunny[numVertices_bunny];
int triangle_list_bunny[numTriangles_bunny * 3];

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
        triangle_list_bunny[(3 * i) + 1] = y;
        triangle_list_bunny[(3 * i) + 2] = z;
    }
    file.close();
}
void calculateNormalsForBunny() {
    for (int i = 0; i < numVertices_bunny -2 ; i++) {
        point4 a = vertex_list_bunny[i];
        point4 b = vertex_list_bunny[i+1];
        point4 c = vertex_list_bunny[i+2];
        vec3 normal = normalize(cross(b - a, c - b));
        normals_bunny[i] = normal;
    }
}

// Retrieved from class notes.
GLubyte*
readPPM(const char* filename) {
    int n, m;
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
    GLubyte* image_temp = (GLubyte*) malloc(3 * sizeof(GLuint) * nm);
    for (i = nm; i > 0; i--)
    {
        fscanf(fd, "%d %d %d", &red, &green, &blue);
        image_temp[3 * nm - 3 * i] = red;
        image_temp[3 * nm - 3 * i + 1] = green;
        image_temp[3 * nm - 3 * i + 2] = blue;
    }
    return image_temp;
    
}


vec2 calculate_u_v(point4 point) {
    GLfloat u, v;
    point4 V = normalize(point);
    double r = scale;
    
    v = acos(V.z / r) / M_PI;
    u = acos(V.x / (r * sin(M_PI * (v)))) / (2 * M_PI);
    return vec2(u, v);
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

    vec3 normal = normalize(cross(b - a, c - b));

    
    //TODO: add quads and textures.
    colors_sphere[Index_sphere] = color;  tex_coords[Index_sphere] = calculate_u_v(a); normals_sphere[Index_sphere] = normal; points_sphere[Index_sphere] = a; Index_sphere++;
    colors_sphere[Index_sphere] = color;  tex_coords[Index_sphere] = calculate_u_v(b); normals_sphere[Index_sphere] = normal; points_sphere[Index_sphere] = b; Index_sphere++;
    colors_sphere[Index_sphere] = color;  tex_coords[Index_sphere] = calculate_u_v(c); normals_sphere[Index_sphere] = normal; points_sphere[Index_sphere] = c; Index_sphere++;
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
GLuint  ModelView, Projection, vPosition, vNormal, Shading_Mode, isLightSourceFixed;
GLuint customTexture, program, Drawing_Type, Texture_Type;
vec4 zero = vec4(0, 0, 0, 0);


// Initialize shader lighting parameters
point4 light_position(0.0, 0.0, -2.0, 1.0); //point light source. GOOD FOR DISPLAYING FIXED LIGHT SOURCE
//point4 light_position(0.0, 0.0, 2.0, 1.0); // GOOD FOR DISPLAYING LIGHT SOURCE MOVING WITH OBJECT
color4 light_ambient(0.2, 0.2, 0.2, 1.0);
color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
color4 light_specular(1.0, 1.0, 1.0, 1.0);

color4 material_ambient(1.0, 0.0, 1.0, 1.0);
color4 material_diffuse(1.0, 0.8, 0.0, 1.0);
color4 material_specular(1.0, 0.8, 0.0, 1.0);
float material_shininess = PLASTIC;


color4 ambient_product = light_ambient * material_ambient;
color4 diffuse_product = light_diffuse * material_diffuse;
color4 specular_product = light_specular * material_specular;


//----------------------------------------------------------------------------
void setProjection(void) {
    mat4  projection;
    //projection = Ortho(-projection_constant * widthRatio, projection_constant * widthRatio, -projection_constant * heightRatio, projection_constant * heightRatio, -projection_constant, projection_constant); // Ortho(): user-defined function in mat.h
    projection = Perspective(45.0, scale, 0.5, 5.0);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
}
// OpenGL initialization
void
init()
{
    readBunny(); // read bunny and extract triangles and vertex list, which will then used for bunny creation.
    calculateNormalsForBunny();
    for (int i = 0; i < numVertices_bunny; i++) {
        colors_bunny[i] = color; // Assign bunny colors at initialization. (to black)
    }
    tetrahedron_sphere(NumTimesToSubdivide); // Create sphere.
    
    objectLocation.initObjectLocation((-projection_constant + scale + 0.2)/5, (scale+0.3)/5, velocityConst, -2*velocityConst);

    ///////////////////////
    // Texture
    // Initialize texture objects
    image = readPPM("basketball.ppm");
    image2 = readPPM("earth.ppm");
    glGenTextures(2, textures);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureSizeX, TextureSizeY, 0,
        GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //try here different alternatives
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //try here different alternatives
    // enable automatic texture coordinate generation
    


    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureSizeX_2, TextureSizeY_2, 0,
        GL_RGB, GL_UNSIGNED_BYTE, image2);
    glGenerateMipmap(GL_TEXTURE_2D); // try also activating mipmaps for the second texture object
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]); //set current texture
    ////////////////////////
    /////////////////////



    glGenVertexArrays( 2, vao );
    glBindVertexArray( vao[0] );
    program = InitShader("vshader.glsl", "fshader.glsl");
    
    // Sphere object binding and creation.
    glGenBuffers(1, &buffer_sphere);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_sphere);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(normals_sphere) + sizeof(colors_sphere) + sizeof(tex_coords), NULL, GL_STATIC_DRAW);

    GLintptr offset = 0;

    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(points_sphere), points_sphere);
    offset += sizeof(points_sphere);
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(normals_sphere), normals_sphere);
    offset += sizeof(normals_sphere);
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(tex_coords), tex_coords);
    offset += sizeof(tex_coords);
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(colors_sphere), colors_sphere);
    
    
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

    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));
    offset += sizeof(tex_coords);

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));


    /////
    // Bunny object binding and creation.
    glBindVertexArray(vao[1]);
    glGenBuffers(1, &buffer_bunny);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_bunny);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list_bunny) + sizeof(normals_bunny) + sizeof(colors_bunny), NULL, GL_STATIC_DRAW);

    offset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vertex_list_bunny), vertex_list_bunny);
    offset += sizeof(vertex_list_bunny);
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(normals_bunny), normals_bunny);
    offset += sizeof(normals_bunny);
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(colors_bunny), colors_bunny);

    GLuint index_buffer_bunny;
    glGenBuffers(1, &index_buffer_bunny);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_bunny);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangle_list_bunny), triangle_list_bunny, GL_STATIC_DRAW);

    offset = 0;
    GLuint vPosition_bunny = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition_bunny);
    glVertexAttribPointer(vPosition_bunny, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));
    offset += sizeof(vertex_list_bunny);

    GLuint vNormal_bunny = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal_bunny);
    glVertexAttribPointer(vNormal_bunny, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
    offset += sizeof(normals_bunny);

    GLuint vColor_bunny = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor_bunny);
    glVertexAttribPointer(vColor_bunny, 4, GL_FLOAT, GL_FALSE, 0,
        BUFFER_OFFSET(offset));


    //////////////////////



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

    Shading_Mode = glGetUniformLocation(program, "Shading_Mode");
    Drawing_Type = glGetUniformLocation(program, "Drawing_Type");
    Texture_Type = glGetUniformLocation(program, "Texture_Type");
    isLightSourceFixed = glGetUniformLocation(program, "isLightSourceFixed");

    glUniform1i(Shading_Mode, shading_mode);
    glUniform1i(Drawing_Type, drawing_type);
    glUniform1i(Texture_Type, texture_type);
    glUniform1i(isLightSourceFixed, isFixed);

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

void
display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 model_view;
    const vec3 displacement(objectLocation.locX, objectLocation.locY, -2.5);
    GLintptr offset = 0;

    switch (object_type) {

    case ObjectType::SPHERE:
        for (int i = 0; i < NumVertices_sphere; i++) {
            colors_sphere[i] = color; // Reassign colors for sphere. And then rebind the points and color data to reflect the color changes.
        }

        glBindVertexArray(vao[0]);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_sphere);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points_sphere) + sizeof(normals_sphere) + sizeof(colors_sphere) + sizeof(tex_coords), NULL, GL_STATIC_DRAW);

        offset = 0;

        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(points_sphere), points_sphere);
        offset += sizeof(points_sphere);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(normals_sphere), normals_sphere);
        offset += sizeof(normals_sphere);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(tex_coords), tex_coords);
        offset += sizeof(tex_coords);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(colors_sphere), colors_sphere);


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
        case TEXTURE:
            glDrawArrays(GL_TRIANGLES, 0, NumVertices_sphere);
            break;

        }
        
        //model_view = (Translate(displacement) * Scale(scale / 5, scale / 5, scale / 5)) * RotateY(Theta[Yaxis]);
        model_view = (Translate(displacement) * Scale(scale/5, scale/5, scale/5)) * RotateZ(Theta[Zaxis]);
        break;
    case ObjectType::BUNNY:
        for (int i = 0; i < numVertices_bunny; i++) {
            colors_bunny[i] = color; // Reassign colors for bunny. And then rebind the points and color data to reflect the color changes.
        }
        glBindVertexArray(vao[1]);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_bunny);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list_bunny) + sizeof(normals_bunny) + sizeof(colors_bunny), NULL, GL_STATIC_DRAW);

        offset = 0;
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vertex_list_bunny), vertex_list_bunny);
        offset += sizeof(vertex_list_bunny);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(normals_bunny), normals_bunny);
        offset += sizeof(normals_bunny);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(colors_bunny), colors_bunny);


        switch (drawing_type)
        {
        case WIREFRAME:
            //glDrawArrays(GL_LINE_LOOP, 0, numVertices_bunny);
            glDrawElements(GL_LINE_LOOP, numTriangles_bunny * 3, GL_UNSIGNED_INT, 0);

            break;
        case SHADING:
            //glDrawArrays(GL_TRIANGLES, 0, numVertices_bunny);
            glDrawElements(GL_TRIANGLES, numTriangles_bunny * 3, GL_UNSIGNED_INT, 0);
            break;

        }
        model_view = (Translate(displacement) * Scale(scale / 50, scale / 50, scale / 50)) * RotateZ(Theta[Yaxis]);
        //model_view = (Translate(displacement) * Scale(scale/50, scale/50, scale/50)) * RotateZ(Theta[Zaxis]);


    }

    
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
    
    setProjection();
    
    glutSwapBuffers();
    
}

void reshape( int w, int h )
{
    glViewport( 0, 0, w, h );
    scale = (GLfloat)w / (GLfloat)h;
    widthRatio = (GLfloat)w / 760;
    heightRatio = (GLfloat)h / 760;
}


void
keyboard( unsigned char key,int x, int y )
{
    switch (key) {
    case 'q': case 'Q':
        exit(EXIT_SUCCESS);
        break;
    case 'h': case 'H':
        cout << "-h: Help:" << endl << "-q: Quit program" << endl << "-i: Initilization of the animation, starting from the top left corner" << endl;
        cout << "Right click to open up the menu" << endl << "Allowed Actions:" << endl;
        cout << "Gouraud-Phong-Modified Phong" << endl << "Textures for basketball and earth" << endl << "Material: Metallic and plastic" << endl << "Light source fixed or move with the object" << endl;
        cout << "Sphere or bunny for shading" << endl;
        cout << "Change color with numpad actions from 1-8:" << endl;
        cout << "  1: Black" << endl << "  2: Red" << endl << "  3: Yellow" << endl << "  4: Green" << endl << "  5: Blue" << endl << "  6: Magenta" << endl << "  7: White" << endl << "  8: Cyan" << endl;
        break;
    case 'i': case 'I':
        objectLocation.initObjectLocation((-projection_constant + 2*scale)/5, (scale + 0.1)/5, velocityConst, -2 * velocityConst);
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

void timer( int p )
{
    // Update the object location at each time passed. The radius are updated since there can be change in object types and objects do not have the same scale.
    // Thus prevents resulting in unwanted scale problems.
    switch (object_type) {
    case ObjectType::SPHERE:
        objectLocation.updateObjectLocation(scale/5, scale/5, widthRatio, heightRatio);
        break;
    case ObjectType::BUNNY:
        objectLocation.updateObjectLocation(scale/10, scale/10 * 1.8, widthRatio, heightRatio);
        break;

    };
    // GOOD FOR DISPLAYING MOVING WITH OBJECT.
    /*Theta[Yaxis] += 0.5;
    if (Theta[Yaxis] > 360.0) {
        Theta[Yaxis] -= 360.0;
    }*/
    // GOOD FOR FIXED LIGHTING.
    Theta[Zaxis] += 3;
    if (Theta[Zaxis] > 360.0) {
        Theta[Zaxis] -= 360.0;
    }
    glutPostRedisplay();
    
    glutTimerFunc(2,timer,0);
}

void menuStart(int id) {

}

void textureChoice(int id) {
    switch (id) {
    case 1:
        texture_type = BASKETBALL;
        glBindTexture(GL_TEXTURE_2D, textures[0]); //set current texture
        break;
    case 2:
        texture_type = EARTH;
        glBindTexture(GL_TEXTURE_2D, textures[1]); //set current texture
        break;
    }
    glUniform1i(Texture_Type, texture_type);
    
    glutPostRedisplay();
}


void objectChoice(int id) {
    switch (id) {
    case 1:
        object_type = ObjectType::SPHERE;
        break;
    case 2:
        object_type = ObjectType::BUNNY;
        break;
    }

    glutPostRedisplay();
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
        material_shininess = PLASTIC;
        break;
    case 2:
        // Metallic 100-200
        material_shininess = METALLIC;
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
    int shading_menu, shading_component, light_source, material_property, display_mode, texture_menu_type, object_menu_type;
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


    texture_menu_type = glutCreateMenu(textureChoice);
    glutAddMenuEntry("Basketball", 1);
    glutAddMenuEntry("Earth", 2);

    object_menu_type = glutCreateMenu(objectChoice);
    glutAddMenuEntry("Sphere", 1);
    glutAddMenuEntry("Bunny", 2);

    // Main Menu
    glutCreateMenu(menuStart);
    glutAddSubMenu("Shading", shading_menu);
    glutAddSubMenu("Shading Component", shading_component);
    glutAddSubMenu("Material", material_property);
    glutAddSubMenu("Light Source", light_source);
    glutAddSubMenu("Display Mode", display_mode);
    glutAddSubMenu("Texture Type", texture_menu_type);
    glutAddSubMenu("Object Type", object_menu_type);
    
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

