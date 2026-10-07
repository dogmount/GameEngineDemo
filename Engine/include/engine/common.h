#ifndef ENGINE_COMMON_H
#define ENGINE_COMMON_H
#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace GL_engine
{
    class Texture;

    //  相机数据
    struct CameraData
    {
        glm::mat4 viewMatrix; //  视图矩阵

        glm::mat4 projectionMatrix; //  投影矩阵

        glm::mat4 orthoMatrix; //  2D正交投影

        glm::vec3 position; //  位置
    };

    //  阴影相机数据
    struct ShadowCamera
    {
        CameraData cameraData;                //  光视角相机数据
        glm::mat4 viewProj = glm::mat4(1.0f); //  光视角矩阵(投影*视图) 供材质反算阴影坐标

        static ShadowCamera DirectionalLight(const glm::vec3 &direction, const glm::vec3 &sceneCenter, float sceneRadius)
        {
            ShadowCamera shadowCamera;

            const glm::vec3 LightDir = glm::normalize(direction);
            const float margin = sceneRadius * 0.1f;        //  额外边界, 避免阴影边缘被裁剪
            const float radius = sceneRadius + margin;      //  场景包围球半径+边界
            const float dist = sceneRadius * 2.0f + margin; //  光源距离(足够远, 保证光线平行) 相机在场景外，避免近裁剪面裁掉阴影

            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);   //  上方向
            if (glm::abs(glm::dot(LightDir, up)) > 0.99f) //  光线与上方向接近平行(替换上方向，避免视图矩阵不稳定)
            {
                up = glm::vec3(0.0f, 0.0f, 1.0f); //  垂直光防 lookAt 退化
            }

            const glm::vec3 eye = sceneCenter + LightDir * dist;                                                                   //  光视角
            shadowCamera.cameraData.viewMatrix = glm::lookAt(eye, sceneCenter, up);                                                //  光视角矩阵
            shadowCamera.cameraData.projectionMatrix = glm::ortho(-radius, radius, -radius, radius, dist - radius, dist + radius); //  光视角正交投影矩阵(near/far 需覆盖场景包围球)
            shadowCamera.cameraData.position = eye;                                                                                //  光视角相机位置
            shadowCamera.viewProj = shadowCamera.cameraData.projectionMatrix * shadowCamera.cameraData.viewMatrix;                 //  光视角矩阵(投影*视图) 供材质反算阴影坐标
            return shadowCamera;
        }
    };

    //  光照数据
    struct LightData
    {
        glm::vec3 color;    // 光照颜色
        glm::vec3 position; // 光照位置

        //  阴影
        bool castsShadow = false; // 是否投射阴影
        ShadowCamera shadow;      // 阴影相机数据
    };

    //  UI批次
    struct UIBatch
    {
        Texture *texture = nullptr;
        uint32_t indexCount = 0;
    };
}

#endif