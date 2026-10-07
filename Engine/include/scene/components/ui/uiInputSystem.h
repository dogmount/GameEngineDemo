#ifndef UI_INPUT_SYSTEM_H
#define UI_INPUT_SYSTEM_H
#pragma once

#include <vector>

namespace GL_engine
{
    class UIElementComponent;
    class CanvasComponent;

    class UIInputSystem
    {
    public:
        void Update(float deltaTime);

        //  设置/获取活动状态
        void SetActive(bool active);
        bool IsActive() const;

        //  设置/获取UI画布
        void SetCanvas(CanvasComponent *canvas);
        CanvasComponent *GetCanvas();

        //  获取UI
        std::vector<UIElementComponent *> CollectUI(CanvasComponent *canvas);

    private:
        bool m_active = false;
        CanvasComponent *m_activeCanvas = nullptr; //  活动画布
        UIElementComponent *m_hovered = nullptr;   //  悬停UI对象
        UIElementComponent *m_pressed = nullptr;   //  按下UI对象
    };
}

#endif