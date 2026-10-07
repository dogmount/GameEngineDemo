#include "scene/components/lightComponent.h"

namespace GL_engine
{
    //  加载属性
    void LightComponent::LoadProperties(const nlohmann::json &json)
    {
        if (json.contains("color"))
        {
            const auto &colorObj = json["color"];
            glm::vec3 color(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f));

            SetColor(color);
        }

        m_castShadow = json.value("castShadow", m_castShadow);

        if (json.contains("shadowCenter"))
        {
            const auto &c = json["shadowCenter"];
            m_shadowCenter = glm::vec3(c.value("x", 0.0f), c.value("y", 0.0f), c.value("z", 0.0f));
        }
        m_shadowRadius = json.value("shadowRadius", m_shadowRadius);
    }

    void LightComponent::Update(float deltaTime)
    {
    }

    //  设置/获取颜色
    void LightComponent::SetColor(const glm::vec3 &color)
    {
        m_color = color;
    }
    const glm::vec3 &LightComponent::GetColor() const
    {
        return m_color;
    }

    //  设置/获取阴影投射
    void LightComponent::SetCastsShadow(bool enable)
    {
        m_castShadow = enable;
    }
    bool LightComponent::GetCastsShadow() const
    {
        return m_castShadow;
    }

    //  设置/获取阴影中心点
    void LightComponent::SetShadowCenter(const glm::vec3 &center)
    {
        m_shadowCenter = center;
    }
    const glm::vec3 &LightComponent::GetShadowCenter() const
    {
        return m_shadowCenter;
    }

    //  设置/获取阴影半径
    void LightComponent::SetShadowRadius(float radius)
    {
        m_shadowRadius = radius;
    }
    float LightComponent::GetShadowRadius() const
    {
        return m_shadowRadius;
    }
}