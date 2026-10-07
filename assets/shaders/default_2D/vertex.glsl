#version 330 core
layout(location = 0) in vec2 position;

out vec2 vUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

            //  中心点/尺寸
uniform vec2 uPivot;
uniform vec2 uSize;

            //  左下/右上UV坐标
uniform vec2 uUVmin;
uniform vec2 uUVmax;

void main() {
    vec2 local = (position - uPivot) * uSize;
    vUV = mix(uUVmin, uUVmax, position);

    gl_Position = uProjection * uView * uModel * vec4(local, 0.0, 1.0);
}