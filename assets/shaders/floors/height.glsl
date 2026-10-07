#version 330 core

struct Light {
    vec3 color;
    vec3 direction;
    vec3 position;
};

uniform Light uLight;
uniform vec3 uCameraPos;
uniform float uvScale;
uniform float heightScale; //  视差强度(UV 单位): ≈ 真实起伏深度(米) / uvScale

in vec2 vUV;
in vec3 vNormal;
in vec3 vFragPos;
in vec4 vTangent;

uniform sampler2D diffuseTexture;
uniform sampler2D normalMap;
uniform sampler2D heightMap;   //  高度图: 白=凸(砖面), 黑=凹(砖缝)

out vec4 FragColor;

//  ---------- 高度场采样(方案B: 专用高度图) ----------
float SampleHeight(vec2 p) {
    return texture(heightMap, p).r;
}

void main() {
    float gamma = 2.2;
    float s = (uvScale > 0.0) ? uvScale : 1.0; //  保护: 未绑定时 uniform 默认是 0.0, 不能裸除
    vec2 uv = vUV / s;

    //  ---------- 切线基(插值后正交化) ----------
    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent.xyz - N * dot(N, vTangent.xyz));
    vec3 B = cross(N, T) * vTangent.w;

    //  ---------- 视差: 视线 → 切线空间 ----------
    vec3 viewDirWS = normalize(uCameraPos - vFragPos);
    vec3 V = vec3(dot(viewDirWS, T), dot(viewDirWS, B), dot(viewDirWS, N));
    //  V.z = 垂直表面分量; V.xy = 沿表面分量 → 越斜看偏移越大

    //  ---------- POM: 陡峭视差步进 + 跨层插值 ----------
    const float kMinLayers = 8.0;
    const float kMaxLayers = 32.0;
    float numLayers = mix(kMaxLayers, kMinLayers, abs(V.z)); //  正视省采样, 斜视加采样
    float layerDepth = 1.0 / numLayers;

    //  ❈ 若凹凸方向反了(凸起变凹坑): 把下面 -= 改成 +=, 或给 heightScale 取负
    vec2 P = (V.xy / max(abs(V.z), 1e-3)) * heightScale;
    vec2 dUV = P / numLayers;

    vec2 uvPOM = uv;
    float layer = 0.0;
    float hCur = SampleHeight(uvPOM);

    //  沿视线反向推移 UV, 直到"钻入"高度场之下 → 找到真实可见点
    while(layer < hCur) {
        uvPOM -= dUV;
        hCur = SampleHeight(uvPOM);
        layer += layerDepth;
    }

    //  跨层插值(消除阶梯感)
    vec2 uvPrev = uvPOM + dUV;
    float hPrev = SampleHeight(uvPrev);
    float afterDepth = hCur - layer;                  //  当前层穿入深度(通常为负)
    float beforeDepth = hPrev - (layer - layerDepth);  //  上一层残留高度(通常为正)
    float weight = afterDepth / (afterDepth - beforeDepth + 1e-5);
    uvPOM = mix(uvPOM, uvPrev, clamp(weight, 0.0, 1.0));

    //  ---------- 用视差后的 UV 采样表面(两个采样都必须用 uvPOM) ----------
    vec3 color = pow(texture(diffuseTexture, uvPOM).rgb, vec3(gamma));
    vec3 n = texture(normalMap, uvPOM).rgb * 2.0 - 1.0;
    vec3 normal = normalize(mat3(T, B, N) * n);        //  切线空间 → 世界空间

    //  ---------- 光照(与原来一致) ----------
    float strength = 0.1;
    vec3 ambient = strength * color;

    vec3 lightDir = normalize(-uLight.direction);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    vec3 halfwayDir = normalize(lightDir + viewDirWS);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = vec3(0.3) * spec;

    vec3 result = (ambient + diffuse + specular);
    FragColor = vec4(pow(result, vec3(1.0 / gamma)), 1.0);
}