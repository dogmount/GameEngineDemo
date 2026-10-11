#version 330 core
out vec4 FragColor;
in vec2 outTexCoord;

uniform vec3 lightColor;
uniform float intensity;   //  新增: 自发光强度(HDR, >1 才会被泛光链抓到)

void main() {
    FragColor = vec4(lightColor * intensity, 1.0);
}