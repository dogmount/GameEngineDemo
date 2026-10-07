#version 330 core

in vec2 vUV;

uniform vec4 uColor;

uniform sampler2D uTex;

out vec4 FragColor;

void main() {
    vec4 src = texture(uTex, vUV) * uColor;
    FragColor = src;
}