#version 330 core

layout(location = 0) in vec3 aPos;   //  立方体顶点位置 —— 直接当采样方向用

out vec3 vDir;

uniform mat4 uView;         //  C++ 侧已去掉平移(纯旋转)
uniform mat4 uProjection;

void main() {
    vDir = aPos;            //  盒中心在原点且等于世界原点, 顶点位置即世界方向

    //  z 压到远平面: 背景像素(深度=1.0)通过, 已有几何的像素失败
    vec4 viewPos = uView * vec4(aPos, 1.0);
    gl_Position = uProjection * viewPos;
    gl_Position.z = gl_Position.w * 0.9999;   //  NDC z ≈ 0.9999 (深度测试用 LESS)
}