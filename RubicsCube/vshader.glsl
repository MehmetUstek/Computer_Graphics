#version 410

in vec4 vPosition;
in vec4 vColor;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 vRotation[8];
uniform vec4 cubeColor[8];

uniform int currentCube;
uniform int edge;
uniform int selectedFace;
uniform int selectedCube;

out vec4 color;

void setColor() {
    if (selectedCube == 1) {
        color = cubeColor[currentCube];
    }
    else if (edge == 1) {
        color = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else {
        color = vColor;
    }
}

void setPosition() {
    if (selectedFace == 1) {
        gl_Position = Projection * ModelView * vPosition;
    }
    else {
        gl_Position = Projection * ModelView * vRotation[currentCube] * vPosition;
    }
}

void main() {
    setPosition();
    setColor();
}
