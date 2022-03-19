#ifndef OBJECT_H
#define OBJECT_H
#include "Angel.h"

typedef vec4  color4;
typedef vec4  point4;

class Object {
public:
    int numOfVertices;
    point4 points;
    color4 colors;
    color4 RGB;
    color4 color = color4(1.0, 0.0, 0.0, 1.0);

    virtual void create();
    virtual void change_color(color4);
    virtual void change_color_rgb();
};
#endif