#ifndef LIGHT_COMPONENT_H
#define LIGHT_COMPONENT_H
#pragma once

#include "scene/component.h"

#include <glm/glm.hpp>

namespace GL_engine
{
    class LightComponent : public Component
    {
        COMPONENT(LightComponent)
    public:
        //  加载属性
        void LoadProperties(const nlohmann::json &json) override;

        void Update(float deltaTime) override;

        //  设置/获取颜色
        void SetColor(const glm::vec3 &color);
        const glm::vec3 &GetColor() const;

        //  设置/获取阴影投射
        void SetCastsShadow(bool enable);
        bool GetCastsShadow() const;

        //  设置/获取阴影中心点
        void SetShadowCenter(const glm::vec3 &center);
        const glm::vec3 &GetShadowCenter() const;

        //  设置/获取阴影半径
        void SetShadowRadius(float radius);
        float GetShadowRadius() const;

    private:
        glm::vec3 m_color = glm::vec3(1.0f);

        //  阴影相关
        bool m_castShadow = false;                 // 是否投射阴影
        glm::vec3 m_shadowCenter = glm::vec3(0.0f); // 阴影中心点
        float m_shadowRadius = 60.0f;               // 阴影半径
    };
}

#endif