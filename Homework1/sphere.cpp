#include "Angel.h"


const int NumTimesToSubdivide = 5;
const int NumTriangles = 4096;
// (4 faces)^(NumTimesToSubdivide + 1)
const int NumVertices_sphere = 3 * NumTriangles;
typedef Angel::vec4 point4;
typedef Angel::vec4 color4;
point4 points_sphere[NumVertices_sphere];
vec3 normals_sphere[NumVertices_sphere];


int Index = 0;
void
triangle(const point4& a, const point4& b, const point4& c)
{
	vec3 normal = normalize(cross(b - a, c - b));
	normals_sphere[Index] = normal; points_sphere[Index] = a; Index++;
	normals_sphere[Index] = normal; points_sphere[Index] = b; Index++;
	normals_sphere[Index] = normal; points_sphere[Index] = c; Index++;
}

point4
unit(const point4& p)
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
divide_triangle(const point4& a, const point4& b,
	const point4& c, int count)
{
	if (count > 0) {
		point4 v1 = unit(a + b);
		point4 v2 = unit(a + c);
		point4 v3 = unit(b + c);
		divide_triangle(a, v1, v2, count - 1);
		divide_triangle(c, v2, v3, count - 1);
		divide_triangle(b, v3, v1, count - 1);
		divide_triangle(v1, v3, v2, count - 1);
	}
	else {
		triangle(a, b, c);
	}
}

void
tetrahedron(int count)
{
	point4 v[4] = {
	vec4(0.0, 0.0, 1.0, 1.0),
	vec4(0.0, 0.942809, -0.333333, 1.0),
	vec4(-0.816497, -0.471405, -0.333333, 1.0),
	vec4(0.816497, -0.471405, -0.333333, 1.0)
	};
	divide_triangle(v[0], v[1], v[2], count);
	divide_triangle(v[3], v[2], v[1], count);
	divide_triangle(v[0], v[3], v[1], count);
	divide_triangle(v[0], v[2], v[3], count);
}
