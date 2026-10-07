#include "engine/Application.h"

namespace GL_engine
{
    void Application::RegisterTypes()
    {
    }

    // 设置关闭信号
    void Application::SetAppToClose(bool value)
    {
        m_AppToClose = value;
    }

    bool Application::ShouldAppToClose() const
    {
        return m_AppToClose;
    }
}