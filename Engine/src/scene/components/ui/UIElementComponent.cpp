#include "scene/components/ui/UIElementComponent.h"

namespace GL_engine
{
    void UIElementComponent::Render(CanvasComponent *canvas)
    {
    }

    //  获取鼠标与UI交互状态
    bool UIElementComponent::HitTest(const glm::vec2 &pos) const
    {
        return false;
    }

    //  进入UI交互区
    void UIElementComponent::OnPointerEnter()
    {
    }
    //  离开UI交互区
    void UIElementComponent::OnPointerExit()
    {
    }
    //  UI按下
    void UIElementComponent::OnPointerUp()
    {
    }
    //  UI松开
    void UIElementComponent::OnPointerDown()
    {

    } //  UI点击
    void UIElementComponent::OnClick()
    {
    }
}