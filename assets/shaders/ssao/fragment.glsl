#version 330 core

uniform sampler2D uDepth;   //  depthPrepass 的深度纹理(按 inputBindings 自动绑定)
uniform mat4 uProjection;   //  bindCameraUniforms 自动注入

//  可调参数(先写死, 要调到手软再考虑挪成 uniform)
const float RADIUS = 0.6;    //  采样半球半径(世界单位≈米), 覆盖墙角/箱底缝隙
const float BIAS = 0.025;  //  自遮蔽偏移: 太小 → 平坦面自己遮自己变脏
const float POWER = 2.0;    //  对比度: 越大暗部越深
const int KERNEL_SIZE = 16;

in vec2 vUV;
layout(location = 0) out vec4 FragColor;

//  黄金角螺旋半球采样点: 均匀且不需要 uniform 数组(引擎 SetUniform 没有数组重载)
vec3 KernelSample(int i) {
    float t = (float(i) + 0.5) / float(KERNEL_SIZE);   //  0~1
    float phi = float(i) * 2.3999632;                  //  黄金角
    float r = sqrt(t);                                 //  面积均匀
    return vec3(cos(phi) * r * 0.85, sin(phi) * r * 0.85, mix(0.1, 0.7, t)); // z>0 半球
}

//  逐像素哈希 → 随机旋转角(把采样图案打散, 模糊后是细噪而不是规则条纹)
float Hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    //  1) 背景(无几何) → 无遮蔽
    float depth = texture(uDepth, vUV).r;
    if(depth >= 1.0) {
        FragColor = vec4(1.0);
        return;
    }

    //  2) 重建本像素的视空间位置
    mat4 invProj = inverse(uProjection);
    vec4 clip = vec4(vUV * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos4 = invProj * clip;
    vec3 pos = viewPos4.xyz / viewPos4.w;

    //  3) 由屏幕导数重建法线(不需要法线附件)
    vec3 normal = normalize(cross(dFdx(pos), dFdy(pos)));
    if(abs(normal.z) < 0.1) {
        FragColor = vec4(1.0);
        return;
    } // 几乎侧对相机的面跳过, 防噪

    //  4) 随机切向 → TBN(每像素旋转核样本)
    float ang = Hash12(gl_FragCoord.xy) * 6.2831853;
    vec3 randomVec = vec3(cos(ang), sin(ang), 0.0);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    //  5) 半球采样 → 遮挡测试
    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; ++i) {
        vec3 samplePos = pos + TBN * KernelSample(i) * RADIUS;

        //  采样点投影回屏幕
        vec4 offset = uProjection * vec4(samplePos, 1.0);
        vec2 sampleUV = (offset.xy / offset.w) * 0.5 + 0.5;
        if(sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0)
            continue;

        float sampleDepth = texture(uDepth, sampleUV).r;
        if(sampleDepth >= 1.0)
            continue; // 采样点落在背景 → 不算遮挡

        //  重建该像素处表面的视空间 z(相机朝 -Z: 值越大越靠近相机)
        vec4 sdClip = vec4(sampleUV * 2.0 - 1.0, sampleDepth * 2.0 - 1.0, 1.0);
        vec4 sdView = invProj * sdClip;
        float sampleViewZ = sdView.z / sdView.w;

        //  可见表面比采样点更靠前 → 采样点被挡
        float blocked = (sampleViewZ >= samplePos.z + BIAS) ? 1.0 : 0.0;
        //  距离衰减: 很远的表面不该遮蔽近处采样点(防漏光)
        float rangeCheck = smoothstep(0.0, 1.0, RADIUS / max(abs(pos.z - sampleViewZ), 1e-4));
        occlusion += blocked * rangeCheck;
    }

    float ao = 1.0 - occlusion / float(KERNEL_SIZE);
    ao = pow(clamp(ao, 0.0, 1.0), POWER);
    FragColor = vec4(ao, ao, ao, 1.0); // 目标 R8: 只取 r
}