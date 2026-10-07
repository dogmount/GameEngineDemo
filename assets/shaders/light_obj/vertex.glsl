#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;

out vec2 vUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vUV = uv;
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}