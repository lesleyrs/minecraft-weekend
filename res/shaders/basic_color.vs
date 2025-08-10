#version 300 es

layout (location = 0) in vec3 position;

uniform mat4 m, v, p;

void main() {
    gl_Position = p * v * m * vec4(position, 1.0);
}
