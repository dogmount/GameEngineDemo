#version 330 core
uniform sampler2D uSource;
uniform vec2 uDirection;    // (1,0)=横向  (0,1)=纵向

in vec2 vUV;
out vec4 FragColor;

void main() {
    vec2 texel = 1.0 / vec2(textureSize(uSource, 0));
    const float w[5] = float[](0.227027, 0.194595, 0.121622, 0.054054, 0.016216);

    vec3 result = texture(uSource, vUV).rgb * w[0];
    for(int i = 1; i < 5; ++i) {
        vec2 off = uDirection * texel * float(i);
        result += texture(uSource, vUV + off).rgb * w[i];
        result += texture(uSource, vUV - off).rgb * w[i];
    }
    FragColor = vec4(result, 1.0);
}