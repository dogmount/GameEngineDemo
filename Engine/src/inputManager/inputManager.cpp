#include "inputManager/inputManager.h"

namespace GL_engine
{
    //  设置/获取键盘按键状态
    void InputManager::SetKeyPressed(int key, bool pressed)
    {
        if (key < 0 || key >= static_cast<int>(m_keys.size()))
        {
            return;
        }
        m_keys[key] = pressed;
    }
    bool InputManager::IsKeyPressed(int key)
    {
        if (key < 0 || key >= static_cast<int>(m_keys.size()))
        {
            return false;
        }
        return m_keys[key];
    }

    //  设置/获取鼠标按键状态
    void InputManager::SetMouseButtonPressed(int button, bool pressed)
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeys.size()))
        {
            return;
        }
        m_mouseKeys[button] = pressed;
    }
    bool InputManager::IsMouseButtonPressed(int button)
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeys.size()))
        {
            return false;
        }
        return m_mouseKeys[button];
    }

    //  设置//获取鼠标按键按下状态
    void InputManager::SetMouseButtonWasPressed(int button, bool pressed)
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeysPressed.size()))
        {
            return;
        }
        m_mouseKeysPressed[button] = pressed;
    }
    bool InputManager::WasMouseButtonPressed(int button) const
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeysPressed.size()))
        {
            return false;
        }
        return m_mouseKeysPressed[button];
    }

    //  设置//获取鼠标按键释放状态
    void InputManager::SetMouseButtonWasReleased(int button, bool pressed)
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeysReleased.size()))
        {
            return;
        }
        m_mouseKeysReleased[button] = pressed;
    }
    bool InputManager::WasMouseButtonReleased(int button) const
    {
        if (button < 0 || button >= static_cast<int>(m_mouseKeysReleased.size()))
        {
            return false;
        }
        return m_mouseKeysReleased[button];
    }

    //  设置/获取鼠标光标上一个位置
    void InputManager::SetMousePositionLast(const glm::vec2 &pos)
    {
        m_mousePostitionLast = pos;
    }
    const glm::vec2 &InputManager::GetMousePositionLast() const
    {
        return m_mousePostitionLast;
    }

    //  设置/获取鼠标光标当前位置
    void InputManager::SetMousePositionCurrent(const glm::vec2 &pos)
    {
        m_mousePostionCurrent = pos;
    }
    const glm::vec2 &InputManager::GetMousePositionCurrent() const
    {
        return m_mousePostionCurrent;
    }

    //  设置/判断鼠标移动状态
    void InputManager::SetMousePositionChanged(bool changed)
    {
        m_mousePositionChanged = changed;
    }
    bool InputManager::IsMousePositionChanged() const
    {
        return m_mousePositionChanged;
    }

    //  清理按键状态
    void InputManager::ClearStates()
    {
        SetMousePositionChanged(false);
        for (auto k : m_mouseKeysPressed)
        {
            SetMouseButtonWasPressed(k, false);
        }
        for (auto k : m_mouseKeysReleased)
        {
            SetMouseButtonWasReleased(k, false);
        }
    }
}