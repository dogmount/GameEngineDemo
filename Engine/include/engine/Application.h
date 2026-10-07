#ifndef MY_APPLICATION_H
#define MY_APPLICATION_H
#pragma once

namespace GL_engine
{
    class Application
    {
    public:
        virtual ~Application() = default;

        virtual void RegisterTypes();
        virtual bool AppInit() = 0;                  //  初始化
        virtual void AppUpdate(float deltaTime) = 0; //  每帧更新
        virtual void AppDestroy() = 0;               //  资源回收

        //  设置/获取关闭信号
        void SetAppToClose(bool value);
        bool ShouldAppToClose() const;

    private:
        bool m_AppToClose = false; //  程序关闭信号
    };
}

#endif