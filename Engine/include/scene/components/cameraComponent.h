#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H
#pragma once

#include <glm/glm.hpp>
#include "scene/component.h"

namespace GL_engine
{
    class CameraComponent : public Component
    {
        COMPONENT(CameraComponent)
    public:
        void Update(float deltaTime) override;

        //  获取视图矩阵
        glm::mat4 GetViewMatrix() const;

        //  获取投影矩阵
        glm::mat4 GetProjectionMatrix(float aspect) const;

        //  获取相机位置
        glm::vec3 GetEyePosition() const;

    private:
        float m_fov = 60.0f;
        float m_nearPlane = 0.1f;
        float m_farPlane = 100.0f;
    };
}

#endif