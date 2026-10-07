#version 330 core

in vec2 vUV;
in vec4 vColor;

uniform sampler2D uTex;
uniform int uUserTexture;

out vec4 FragColor;

void main() {
    vec4 src = (uUserTexture != 0) ? texture(uTex, vUV) * vColor : vColor;
    FragColor = src;
}