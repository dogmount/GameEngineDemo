#version 330 core
uniform sampler2D uSSAO;
in vec2 vUV;
layout(location = 0) out vec4 FragColor;

//  4x4 盒式模糊: 抹掉逐像素随机噪点
void main() {
    vec2 texel = 1.0 / vec2(textureSize(uSSAO, 0));
    float sum = 0.0;
    for(int x = -2; x < 2; ++x) {
        for(int y = -2; y < 2; ++y) {
            sum += texture(uSSAO, vUV + vec2(x, y) * texel).r;
        }
    }
    float ao = sum / 16.0;
    FragColor = vec4(ao, ao, ao, 1.0);
}