#include "scene/components/ui/uiInputSystem.h"
#include "scene/components/ui/canvasComponent.h"
#include "scene/components/ui/UIElementComponent.h"
#include "engine/Engine.h"

#include <GLFW/glfw3.h>

namespace GL_engine
{
    void UIInputSystem::Update(float deltaTime)
    {
        if (!m_active || !m_activeCanvas || !m_activeCanvas->IsActive())
        {
            return;
        }

        auto &input = Engine::GetInstance().GetInputManager();
        bool mouseDown = input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        bool mousePressed = input.WasMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        bool mouseReleased = input.WasMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT);

        auto mousePos = input.GetMousePositionCurrent();
        mousePos.y = Engine::GetInstance().GetShaderAPI().GetViewport().height - mousePos.y; //  反转Y(opengl默认左上角)

        UIElementComponent *hit = nullptr;
        auto uiElements = CollectUI(m_activeCanvas);
        for (auto elment : uiElements)
        {
            if (elment->HitTest(mousePos))
            {
                hit = elment;
                break;
            }
        }
        if (hit != m_hovered)
        {
            if (m_hovered)
            {
                m_hovered->OnPointerExit();
            }
            m_hovered = hit;
            if (m_hovered)
            {
                m_hovered->OnPointerEnter();
            }
            m_pressed = nullptr;
        }

        if (!m_pressed)
        {
            if (mousePressed && m_hovered)
            {
                m_pressed = m_hovered;
                m_pressed->OnPointerDown();
            }
        }

        if (mouseReleased)
        {
            if (m_pressed)
            {
                m_pressed->OnPointerUp();
                if (m_pressed == m_hovered)
                {
                    m_pressed->OnClick();
                }
            }
            m_pressed = nullptr;
        }
    }

    //  设置/获取活动状态
    void UIInputSystem::SetActive(bool active)
    {
        m_active = active;
    }
    bool UIInputSystem::IsActive() const
    {
        return m_active;
    }

    //  设置/获取UI画布
    void UIInputSystem::SetCanvas(CanvasComponent *canvas)
    {
        m_activeCanvas = canvas;
    }
    CanvasComponent *UIInputSystem::GetCanvas()
    {
        return m_activeCanvas;
    }

    //  获取UI
    std::vector<UIElementComponent *> UIInputSystem::CollectUI(CanvasComponent *canvas)
    {
        std::vector<UIElementComponent *> result;
        GameObject *canvasObject = canvas->GetOwner();
        const auto &children = canvasObject->GetChildren();

        //  递归遍历子对象
        for (const auto &child : children)
        {
            if (auto component = child->GetComponent<UIElementComponent>())
            {
                canvas->CollectUI(component, result);
            }
        }
        return result;
    }
}