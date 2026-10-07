#ifndef MY_ENGINE_H
#define MY_ENGINE_H
#pragma once

#include <memory>
#include <chrono>

#include "inputManager/inputManager.h"
#include "shaderAPI/shaderAPI.h"
#include "shaderAPI/texture.h"
#include "shaderAPI/FBOManager.h"
#include "render/renderQueue.h"
#include "render/sceneRenderer.h"
#include "render/renderPipeline.h"
#include "scene/scene.h"
#include "IO_file/file_system.h"
#include "physics/physicsManager.h"
#include "audio/audioManager.h"
#include "font/fontManager.h"
#include "scene/components/ui/uiInputSystem.h"

//  声明GLFW结构体
struct GLFWwindow;

namespace GL_engine
{
    class Application;

    class Engine
    {
    public:
        //  设置引擎为单例模式
        static Engine &GetInstance();

    private:
        Engine() = default;
        Engine(const Engine &) = delete;            //  拷贝构造
        Engine(Engine &&) = delete;                 //  移动构造
        Engine &operator=(const Engine &) = delete; //  拷贝赋值
        Engine &operator=(Engine &&) = delete;      //  移动赋值
        ~Engine() = default;

    public:
        bool EngInit(int width, int height); // 引擎初始化
        void EngRun();                       //  引擎主循环
        void EngDestroy();                   //  引擎销毁资源回收

        void SetApp(Application *app); //  创建程序实例
        Application *GetApp();         //  获取程序实例

        //  获取输入管理器
        InputManager &GetInputManager();

        //  获取着色器API
        ShaderAPI &GetShaderAPI();

        //  获取渲染队列
        RenderQueue &GetRenderQueue();

        //  获取文件系统
        FileSystem &GetFileSystem();

        //  获取纹理管理器
        TextureManager &GetTextureManager();

        //  获取物理管理器
        PhysicsManager &GetPhysicsManager();

        //  获取音频管理器
        AudioManager &GetAudioManager();

        //  获取字体管理器
        FontManager &GetFontManager();

        //  获取UI输入
        UIInputSystem &GetUIInputSystem();

        //  设置鼠标回调启用/禁用
        void SetCursorEnabled(bool enabled);

        //  设置/获取场景
        void SetScene(const std::shared_ptr<Scene> &scene);
        const std::shared_ptr<Scene> &GetScene();

        //  =====================================新增1、：管线式渲染相关=======================================================
        RenderPipeline *GetRenderPipeline();
        SceneRenderer &GetSceneRenderer(); // 供游戏层自定义 Pass 复用绘制器
        FBOManager &GetFrameBufferManager();

        //  游戏层自定义渲染 Pass 的便捷接口
        int AddRenderPass(const RenderPass &pass);                                                              //  追加一个 Pass,返回索引
        void ClearRenderPasses();                                                                               //  清空全部 Pass(重建管线用)
        const std::shared_ptr<Texture> GetOutputTexture(const std::string &name, int attachmentIndex = 0) const; //  取 Pass 输出纹理

        //  帧缓冲尺寸变化统一入口(供 GLFW 回调调用)
        void OnFramebufferResize(int width, int height);

        //  =====================================新增2、：物理模拟步长与帧数相关=======================================================
        void SetPhysicsFixedStep(float seconds); // 物理固定步长(默认 1/60)
        void SetMaxPhysicsSteps(int steps);      // 单帧物理步数上限(死亡螺旋保护)
        void SetSwapInterval(int interval);      // 设置交换间隔

    private:
        void SetupDefaultRenderPipeline(); //  构建默认渲染管线(EngInit 时调用)

        std::unique_ptr<Application> m_app; //  程序类指针

        std::chrono::steady_clock::time_point m_lastTime; //  程序时间

        std::shared_ptr<Scene> m_currentScene; //  场景指针

        //  存储窗口为成员变量
        GLFWwindow *m_window = nullptr;

        //  输入管理器
        InputManager m_inputManager;

        //  着色器API
        ShaderAPI m_shaderAPI;

        //  渲染命令队列
        RenderQueue m_renderQueue;

        //  渲染者
        SceneRenderer m_sceneRenderer;

        //  文件系统
        FileSystem m_fileSystem;

        //  纹理管理器
        TextureManager m_textureManager;

        //  物理模拟管理器
        PhysicsManager m_physicsManager;

        //  音频管理器
        AudioManager m_audioManager;

        //  字体管理器
        FontManager m_fontManager;

        //  UI输入系统
        UIInputSystem m_uiInputSystem;

        //  ---- 新增1、：FBO管理器与渲染管线 ----
        FBOManager m_frameBuffers;                        //  FBO 生命周期管理
        std::unique_ptr<RenderPipeline> m_renderPipeline; //  Pass 编排与执行

        //  ---- 新增2、：物理模拟步长与渲染帧数 ----
        //  ---- 时间与物理步进配置 ----
        int m_maxPhysicsSteps = 5;    //  物理: 单帧最多步数
        float m_maxFrameTime = 0.25f; //  单帧最大计入的 delta(防"暂停后疯狂追帧")

        //  ---- 渲染vsync ----
        int m_swapInterval = 1; //  交换间隔；0 = 不锁帧

        //  ---- 帧循环状态 ----
        float m_accumulator = 0.0f; //  物理时间累加器
    };
}

#endif