#version 330 core

struct Light {
    vec3 color;
    vec3 position; //  点光: 光源世界位置(未归一化)
};

uniform Light uLight;
uniform vec3 uCameraPos;
uniform float uvScale;

in vec2 vUV;
in vec3 vNormal;
in vec3 vFragPos;
in vec4 vTangent;

uniform sampler2D diffuseTexture;
uniform sampler2D normalMap;

out vec4 FragColor;

void main() {
    float s = (uvScale > 0.0) ? uvScale : 1.0; //  保护: 未绑定时 uniform 默认是 0.0, 不能裸除
    vec2 uv = vUV / s;

    float gamma = 2.2;
    vec3 color = pow(texture(diffuseTexture, uv).rgb, vec3(gamma));

    //  ---------- 切线 TBN ----------
    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent.xyz - N * dot(N, vTangent.xyz));
    vec3 B = cross(N, T) * vTangent.w;
    vec3 normal = normalize(mat3(T, B, N) * (texture(normalMap, uv).rgb * 2.0 - 1.0));

    //  ---------- 点光 ----------
    vec3 toLight = uLight.position - vFragPos; //  从片元指向光源(保留长度)
    float d2 = dot(toLight, toLight);
    float dist = sqrt(d2);
    vec3 L = toLight / max(dist, 1e-4);        //  单位方向

    //  衰减 = 强度/距离² × R 处平滑归零窗口(避免硬切边)
    //  调参起点(按你 40×100 的场地尺度): intensity 40~80, R 25~30
    const float intensity = 60.0;
    const float R = 28.0;
    float window = clamp(1.0 - d2 / (R * R), 0.0, 1.0);
    float attenuation = intensity / (d2 + 0.5) * window * window;

    vec3 ambient = 0.1 * color;
    float diff = max(dot(L, normal), 0.0);
    vec3 diffuse = uLight.color * diff * color * attenuation;

    vec3 viewDir = normalize(uCameraPos - vFragPos);
    vec3 halfwayDir = normalize(L + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = uLight.color * (0.3 * spec) * attenuation;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(pow(result, vec3(1.0 / gamma)), 1.0);
}