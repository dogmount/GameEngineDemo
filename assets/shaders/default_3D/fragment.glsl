#version 330 core

struct Light {
    vec3 color;
    vec3 direction;
};

uniform Light uLight;
uniform vec3 uCameraPos;

uniform sampler2D baseColorTexture;
uniform sampler2D normalMap;
uniform sampler2D metallicRoughnessTexture;
uniform float uHasNormalMap;    //  1 = 有法线贴图
uniform float uHasRoughnessMap; //  1 = 有金属粗糙度贴图(glTF: G=粗糙度, B=金属度)

in vec2 vUV;
in vec3 vNormal;
in vec3 vFragPos;

out vec4 FragColor;

//  无切线法线贴图: 屏幕空间导数重建 TBN(cotangent frame)
mat3 CotangentFrame(vec3 N, vec3 worldPos, vec2 uv) {
    vec3 dp1 = dFdx(worldPos);
    vec3 dp2 = dFdy(worldPos);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = inversesqrt(max(max(dot(T, T), dot(B, B)), 1e-8)); // 防退化
    return mat3(T * invmax, B * invmax, N);
}

void main() {
    vec3 N = normalize(vNormal);

    //  ---------- 法线贴图 ----------
    if(uHasNormalMap > 0.5) {
        vec3 n = texture(normalMap, vUV).rgb * 2.0 - 1.0;
        mat3 TBN = CotangentFrame(N, vFragPos, vUV);
        N = normalize(TBN * n);
    }

    //  ---------- 粗糙度/金属度 ----------
    float roughness = 0.6;
    float metallic = 0.0;
    if(uHasRoughnessMap > 0.5) {
        vec4 mr = texture(metallicRoughnessTexture, vUV);
        roughness = mr.g;
        metallic = mr.b;
    }

    vec3 albedo = texture(baseColorTexture, vUV).rgb;

    //  ---------- 光照(引擎风格: 平行光 + Blinn-Phong, 用 rough 驱动高光) ----------
    vec3 lightDir = normalize(-uLight.direction);
    vec3 viewDir = normalize(uCameraPos - vFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * uLight.color;

    float shininess = mix(96.0, 6.0, roughness);          // 光滑→锐利, 粗糙→宽钝
    float specStrength = mix(0.6, 0.05, roughness) * mix(1.0, 2.5, metallic);
    float spec = pow(max(dot(N, halfwayDir), 0.0), shininess) * specStrength;
    vec3 specular = spec * uLight.color;

    vec3 ambient = 0.25 * uLight.color;

    FragColor = vec4(albedo * (ambient + diffuse + specular), 1.0);
}