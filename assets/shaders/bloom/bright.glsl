#version 330 core
uniform sampler2D uScene;
uniform float uThreshold;   // 提取阈值(1.0 = 只抓 HDR 自发光)
uniform float uKnee;        // 软过渡宽度

in vec2 vUV;
out vec4 FragColor;

void main() {
    vec3 c = texture(uScene, vUV).rgb;
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722)); // 亮度
    float w = smoothstep(uThreshold, uThreshold + uKnee, l);
    FragColor = vec4(c * w, 1.0);
}