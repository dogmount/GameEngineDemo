#include "scene/components/ui/buttonComponent.h"
#include "scene/components/ui/canvasComponent.h"
#include "scene/gameObject.h"

namespace GL_engine
{
    void ButtonComponent::LoadProperties(const nlohmann::json &json)
    {
        if (json.contains("rect"))
        {
            auto &rectObj = json["rect"];
            SetRect(glm::vec2(rectObj.value("x", 1.0f), rectObj.value("y", 1.0f)));
        }

        if (json.contains("color"))
        {
            auto colorObj = json["color"];
            SetColor(glm::vec4(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f),
                colorObj.value("a", 1.0f)));
        }

        if (json.contains("hovered"))
        {
            auto hoveredObj = json["hovered"];
            SetHoveredColor(glm::vec4(
                hoveredObj.value("r", 1.0f),
                hoveredObj.value("g", 1.0f),
                hoveredObj.value("b", 1.0f),
                hoveredObj.value("a", 1.0f)));
        }

        if (json.contains("pressed"))
        {
            auto pressedObj = json["pressed"];
            SetPressedColor(glm::vec4(
                pressedObj.value("r", 1.0f),
                pressedObj.value("g", 1.0f),
                pressedObj.value("b", 1.0f),
                pressedObj.value("a", 1.0f)));
        }
    }
    void ButtonComponent::Render(CanvasComponent *canvas)
    {
        if (!canvas)
        {
            return;
        }

        auto pos = m_owner->GetWorldPosition2D();
        pos.x -= m_rect.x * m_pivot.x;
        pos.y -= m_rect.y * m_pivot.y;

        canvas->DrawRect(pos, pos + m_rect, *m_currentColor);
    }
    //  获取鼠标与UI交互状态
    bool ButtonComponent::HitTest(const glm::vec2 &pos) const
    {
        auto ownerPos = m_owner->GetWorldPosition2D();
        float x1 = ownerPos.x - m_rect.x * m_pivot.x;
        float y1 = ownerPos.y - m_rect.y * m_pivot.y;
        float x2 = x1 + m_rect.x;
        float y2 = y1 + m_rect.y;

        return (x1 <= pos.x && x2 >= pos.x && y1 <= pos.y && y2 >= pos.y);
    }
    //  进入UI交互区
    void ButtonComponent::OnPointerEnter()
    {
        m_currentColor = &m_hoveredColor;
    }
    //  离开UI交互区
    void ButtonComponent::OnPointerExit()
    {
        m_currentColor = &m_color;
    }
    //  UI按下
    void ButtonComponent::OnPointerUp()
    {
        m_currentColor = &m_hoveredColor;
    }
    //  UI松开
    void ButtonComponent::OnPointerDown()
    {
        m_currentColor = &m_pressedColor;
    } //  UI点击
    void ButtonComponent::OnClick()
    {
        if (onClick)
        {
            onClick();
        }
    }

    //  设置/获取
    void ButtonComponent::SetRect(const glm::vec2 &rect)
    {
        m_rect = rect;
    }
    const glm::vec2 &ButtonComponent::GetRect() const
    {
        return m_rect;
    }

    //  设置/获取颜色
    void ButtonComponent::SetColor(const glm::vec4 &color)
    {
        m_color = color;
    }
    const glm::vec4 &ButtonComponent::GetColror() const
    {
        return m_color;
    }

    //  设置/获取悬停颜色
    void ButtonComponent::SetHoveredColor(const glm::vec4 &color)
    {
        m_hoveredColor = color;
    }
    const glm::vec4 &ButtonComponent::GetHoveredColror() const
    {
        return m_hoveredColor;
    }

    //  设置/获取按下颜色
    void ButtonComponent::SetPressedColor(const glm::vec4 &color)
    {
        m_pressedColor = color;
    }
    const glm::vec4 &ButtonComponent::GetPressedColror() const
    {
        return m_pressedColor;
    }
}