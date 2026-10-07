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

uniform sampler2D uShadowMap;   //  全局注入: shadowPass 的深度纹理
uniform mat4 uLightViewProj;    //  全局注入: 光源视图投影矩阵(光空间变换)
uniform sampler2D uSSAO;        //  全局注入: 模糊后的环境光遮蔽

out vec4 FragColor;

//  ---------- 高度场采样(方案B: 专用高度图) ----------
float SampleHeight(vec2 p) {
    return texture(heightMap, p).r;
}

//  阴影: 光视角投影 + 3x3 PCF, 返回 0(全影)~1(全亮)
float ShadowFactor(vec3 worldPos, vec3 N, vec3 L) {
    vec4 lp = uLightViewProj * vec4(worldPos, 1.0);
    vec3 proj = lp.xyz / lp.w * 0.5 + 0.5;     //  [-1,1] → [0,1]
    if(proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0) {
        return 1.0;                            //  阴影贴图范围外 → 视为照亮
    }

    float bias = max(0.0015 * (1.0 - dot(N, L)), 0.0005);  //  斜率自适应偏移, 治条纹(acne)
    vec2 texel = 1.0 / vec2(textureSize(uShadowMap, 0));
    float lit = 0.0;
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float d = texture(uShadowMap, proj.xy + vec2(x, y) * texel).r;
            lit += (proj.z - bias > d) ? 0.0 : 1.0;
        }
    }
    return lit / 9.0;
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

    //  ---------- 高光: 砖石风(弱 + 宽 + 逐像素变化) ----------
    //  意图: 塑料感来自"强+锐+整面均匀"; 砖=粗糙电介质 → 强度砍小、波瓣变宽、
    //        并用高度图让砖缝(凹处)几乎无高光, 打破整面一致性
    vec3 halfwayDir = normalize(lightDir + viewDirWS);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 12.0); //  64 → 12: 锐利亮斑 → 宽而弱的哑光

    float hRough = SampleHeight(uvPOM);                        //  复用视差后的 UV(砖面高/砖缝低)
    float specMask = mix(0.3, 1.0, hRough);                    //  缝里 0.3x, 砖面 1x
    vec3 specular = vec3(0.05) * spec * specMask;              //  0.3 → 0.05: 高光能量砍到 1/6

    float shadow = ShadowFactor(vFragPos, normal, lightDir); //  normal=世界空间法线(经TBN), lightDir=指向光源

    //  main 的最终光照处改成:
    vec2 screenUV = gl_FragCoord.xy / vec2(textureSize(uSSAO, 0)); // gl_FragCoord 与 FBO 纹理原点都是左下 ✓
    float ao = texture(uSSAO, screenUV).r;
    vec3 result = ambient * ao + (diffuse + specular) * shadow;    // AO 只乘环境光, 与阴影分工

    FragColor = vec4(pow(result, vec3(1.0 / gamma)), 1.0);
    // FragColor = vec4(vec3(ao), 1.0); //  仅显示 AO, 用于调试
}