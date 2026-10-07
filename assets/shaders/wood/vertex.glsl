#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec4 tangent;

out vec2 vUV;
out vec3 vNormal;
out vec3 vFragPos;
out vec4 vTangent;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vUV = uv;
    vFragPos = vec3(uModel * vec4(aPos, 1.0));

    mat3 normalMatrix = transpose(inverse(mat3(uModel)));
    vNormal = normalMatrix * normal;
    vTangent = vec4(normalMatrix * tangent.xyz, tangent.w);
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}