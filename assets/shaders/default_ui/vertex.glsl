#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;

out vec2 vUV;
out vec4 vColor;

uniform mat4 uProjection;

void main() {
    vUV = uv;
    vColor = color;

    gl_Position = uProjection * vec4(position, 0.0, 1.0);
}