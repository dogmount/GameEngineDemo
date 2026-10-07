#ifndef UI_ELEMENT_COMPONENT_H
#define UI_ELEMENT_COMPONENT_H
#pragma once

#include "scene/component.h"

#include <glm/glm.hpp>

namespace GL_engine
{
    class CanvasComponent;

    class UIElementComponent : public Component
    {
        COMPONENT(UIElementComponent)
    public:
        virtual void Render(CanvasComponent *canvas);

        //  获取鼠标与UI交互状态
        virtual bool HitTest(const glm::vec2 &pos) const;

        //  UI交互事件
        virtual void OnPointerEnter(); //  进入UI交互区
        virtual void OnPointerExit();  //  离开UI交互区
        virtual void OnPointerUp();    //  UI按下
        virtual void OnPointerDown();  //  UI松开
        virtual void OnClick();        //  UI点击

    protected:
        glm::vec2 m_pivot = glm::vec2(0.5f);
    };
}

#endif