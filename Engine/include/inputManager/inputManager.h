#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H
#pragma once

#include <array>
#include <glm/glm.hpp>

namespace GL_engine
{
    class InputManager
    {
    private:
        InputManager() = default;
        InputManager(const InputManager &) = delete;            //  拷贝构造
        InputManager(InputManager &&) = delete;                 //  移动构造
        InputManager &operator=(const InputManager &) = delete; //  拷贝赋值
        InputManager &operator=(InputManager &&) = delete;      //  移动赋值

    public:
        //  设置/获取键盘按键状态
        void SetKeyPressed(int key, bool pressed);
        bool IsKeyPressed(int key);

        //  设置/获取鼠标按键状态
        void SetMouseButtonPressed(int button, bool pressed);
        bool IsMouseButtonPressed(int button);

        //  设置//获取鼠标按键按下状态
        void SetMouseButtonWasPressed(int button, bool pressed);
        bool WasMouseButtonPressed(int button) const;

        //  设置//获取鼠标按键释放状态
        void SetMouseButtonWasReleased(int button, bool pressed);
        bool WasMouseButtonReleased(int button) const;

        //  设置/获取鼠标光标上一个位置
        void SetMousePositionLast(const glm::vec2 &pos);
        const glm::vec2 &GetMousePositionLast() const;

        //  设置/获取鼠标光标当前位置
        void SetMousePositionCurrent(const glm::vec2 &pos);
        const glm::vec2 &GetMousePositionCurrent() const;

        //  设置/判断鼠标移动状态
        void SetMousePositionChanged(bool changed);
        bool IsMousePositionChanged() const;

        //  清理按键状态
        void ClearStates();

    private:
        //  仅引擎类可以创建输入管理器对象
        friend class Engine;

        //  键盘数组
        std::array<bool, 512> m_keys = {false};

        //  记录鼠标按键
        std::array<bool, 16> m_mouseKeys = {false};

        //  记录鼠标按键按下
        std::array<bool, 16> m_mouseKeysPressed = {false};

        //  记录鼠标按键释放
        std::array<bool, 16> m_mouseKeysReleased = {false};

        //  记录鼠标光标上一个位置
        glm::vec2 m_mousePostitionLast = glm::vec2(0.0f);

        //  记录鼠标光标当前位置
        glm::vec2 m_mousePostionCurrent = glm::vec2(0.0f);

        //  记录鼠标移动状态
        bool m_mousePositionChanged = false;
    };
}

#endif