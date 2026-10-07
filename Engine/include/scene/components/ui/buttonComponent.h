#ifndef BUTTON_COMPONENT_H
#define BUTTON_COMPONENT_H
#pragma once

#include "scene/components/ui/UIElementComponent.h"

#include <glm/glm.hpp>
#include <functional>

namespace GL_engine
{
    class ButtonComponent : public UIElementComponent
    {
        COMPONENT_RELATE(ButtonComponent, UIElementComponent)
    public:
        void LoadProperties(const nlohmann::json &json) override;
        void Render(CanvasComponent *canvas) override;
        //  获取鼠标与UI交互状态
        bool HitTest(const glm::vec2 &pos) const override;

        //  UI交互事件
        void OnPointerEnter() override; //  进入UI交互区
        void OnPointerExit() override;  //  离开UI交互区
        void OnPointerUp() override;    //  UI按下
        void OnPointerDown() override;  //  UI松开
        void OnClick() override;        //  UI点击

        //  设置/获取矩形
        void SetRect(const glm::vec2 &rect);
        const glm::vec2 &GetRect() const;

        //  设置/获取颜色
        void SetColor(const glm::vec4 &color);
        const glm::vec4 &GetColror() const;

        //  设置/获取悬停颜色
        void SetHoveredColor(const glm::vec4 &color);
        const glm::vec4 &GetHoveredColror() const;

        //  设置/获取按下颜色
        void SetPressedColor(const glm::vec4 &color);
        const glm::vec4 &GetPressedColror() const;

        std::function<void()> onClick;

    private:
        glm::vec2 m_rect;
        glm::vec4 m_color = glm::vec4(1.0f);

        glm::vec4 m_hoveredColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);    //  悬停颜色
        glm::vec4 m_pressedColor = glm::vec4(0.52f, 0.52f, 0.52f, 1.0f); //  按下颜色
        glm::vec4 *m_currentColor = &m_color;                            //  当前颜色
    };
}

#endif