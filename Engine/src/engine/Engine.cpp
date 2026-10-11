#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "engine/Engine.h"
#include "engine/Application.h"
#include "scene/gameObject.h"
#include "scene/component.h"
#include "scene/components/cameraComponent.h"

#include <algorithm>
#include <thread>

namespace GL_engine
{
    //  参数1:窗口  参数2:按键码  参数3:扫描码(省略)  参数4:动作(按下/释放)  参数5:修饰键(省略)
    void keyCallback(GLFWwindow *window, int key, int, int action, int)
    {
        auto &inputManager = GL_engine::Engine::GetInstance().GetInputManager();
        if (action == GLFW_PRESS)
        {
            inputManager.SetKeyPressed(key, true);
        }
        else if (action == GLFW_RELEASE)
        {
            inputManager.SetKeyPressed(key, false);
        }
    }

    //  鼠标按键状态回调
    void mouseButtonCallback(GLFWwindow *window, int button, int action, int)
    {
        auto &inputManager = GL_engine::Engine::GetInstance().GetInputManager();
        if (action == GLFW_PRESS)
        {
            inputManager.SetMouseButtonPressed(button, true);
            inputManager.SetMouseButtonWasPressed(button, true);
        }
        else if (action == GLFW_RELEASE)
        {
            inputManager.SetMouseButtonPressed(button, false);
            inputManager.SetMouseButtonWasReleased(button, true);
        }
    }

    //  回调获取光标位置 （改进点：初次移动鼠标时，画面不瞬移缓存
    void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos)
    {
        auto &inputManager = GL_engine::Engine::GetInstance().GetInputManager();

        //  将当前光标位置设置为旧位置
        inputManager.SetMousePositionLast(inputManager.GetMousePositionCurrent());

        glm::vec2 currentPos(static_cast<float>(xpos), static_cast<float>(ypos));

        //  更新光标位置
        inputManager.SetMousePositionCurrent(currentPos);

        //  设置鼠标移动状态为已改变
        inputManager.SetMousePositionChanged(true);
    }

    //  视口尺寸回调
    void windowSizeCallback(GLFWwindow *window, int width, int height)
    {
        GL_engine::Engine::GetInstance().GetShaderAPI().SetViewport(0, 0, width, height);
    }

    //  =====================================新增1、：管线式渲染相关=======================================================
    //  视口尺寸回调(替换原 windowSizeCallback)
    void framebufferSizeCallback(GLFWwindow *window, int width, int height)
    {
        GL_engine::Engine::GetInstance().OnFramebufferResize(width, height);
    }
    //  =================================================================================================================

    Engine &Engine::GetInstance()
    {
        static Engine instance; //  保证唯一
        return instance;
    }

    bool Engine::EngInit(int width, int height)
    {
        if (!m_app)
        {
            return false;
        }

        Scene::RegisterTypes();
        m_app->RegisterTypes();

        if (!glfwInit())
        {
            return false;
        }

        // 初始化GLFW
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(width, height, "Opengl_Engine", nullptr, nullptr);
        if (m_window == nullptr)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate(); // 释放GLFW占用的资源
            return false;
        }

        // glfwSetWindowSizeCallback(m_window, windowSizeCallback);    //  视口尺寸回调
        glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback); //  【修改】原为 windowSizeCallback

        glfwSetKeyCallback(m_window, keyCallback);                  //  设置键盘回调
        glfwSetMouseButtonCallback(m_window, mouseButtonCallback);  //  设置鼠标按键回调
        glfwSetCursorPosCallback(m_window, cursorPositionCallback); //  设置鼠标位置回调

        glfwMakeContextCurrent(m_window); // 设置这个窗口为当前渲染目标

        //  GLAD初始化
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //  加载系统相关的OpenGL函数
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            glfwTerminate(); // 释放GLFW占用的资源
            return false;
        }

        m_shaderAPI.Init(&m_fileSystem);
        m_shaderAPI.SetViewport(0, 0, width, height);
        m_physicsManager.Init();
        m_audioManager.Init();
        // m_renderQueue.Init();  //  原初始化内容交由m_sceneRenderer负责
        m_fontManager.Init();

        //  =====================================新增1、：管线式渲染相关=======================================================
        m_sceneRenderer.Init();

        m_shaderAPI.LoadShaderFromPath("depth_default", "shaders/depth/vertex.glsl", "shaders/depth/fragment.glsl");
        m_shaderAPI.LoadShaderFromPath("ssao", "shaders/ssao/vertex.glsl", "shaders/ssao/fragment.glsl");
        m_shaderAPI.LoadShaderFromPath("ssao_blur", "shaders/ssao/vertex.glsl", "shaders/ssao/blur.glsl");

        //  组装渲染管线:必须在 m_shaderAPI、m_renderQueue 初始化之后
        m_renderPipeline = std::make_unique<RenderPipeline>(m_shaderAPI, m_renderQueue, m_sceneRenderer, m_frameBuffers);
        SetupDefaultRenderPipeline();
        //  =================================================================================================================

        return m_app->AppInit();
    }

    void Engine::EngRun()
    {
        if (!m_app)
        {
            return;
        }

        //  =====================================新增2、：物理模拟步长与帧数相关=======================================================
        //  初始化限帧(必须在上下文当前时设置 vsync)
        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(m_swapInterval);
        glfwSwapInterval(0);

        m_accumulator = 0.0f; //  物理时间累加器
        //  ========================================================================================================================

        m_lastTime = std::chrono::steady_clock::now(); //  初始化上一时间点

        while (!glfwWindowShouldClose(m_window) && !m_app->ShouldAppToClose())
        {
            // 处理用户输入和窗口事件
            glfwPollEvents();

            //  计算增量时间
            auto nowTime = std::chrono::steady_clock::now();
            float frameTime = std::chrono::duration<float>(nowTime - m_lastTime).count();
            m_lastTime = nowTime;

            //  =====================================新增2、：物理模拟步长与帧数相关=======================================================
            const float fixedTimeStep = m_physicsManager.GetFixedTimeStep();
            frameTime = std::min(frameTime, m_maxFrameTime);

            m_accumulator += frameTime;
            int steps = 0; //  步数
            while (m_accumulator >= fixedTimeStep && steps < m_maxPhysicsSteps)
            {
                m_physicsManager.Step(fixedTimeStep); // 显式推进"一步"
                m_accumulator -= fixedTimeStep;
                ++steps;
            }

            if (steps >= m_maxPhysicsSteps)
            {
                m_accumulator = 0.0f; // 丢弃积压, 避免死亡螺旋
            }

            //  插值系数(0~1): 渲染时刻落在两个物理步之间的比例(需要插值时用)
            const float physicsAlpha = (fixedTimeStep > 0.0f) ? (m_accumulator / fixedTimeStep) : 0.0f;
            //  ========================================================================================================================

            //  更新UI
            if (m_uiInputSystem.IsActive())
            {
                m_uiInputSystem.Update(frameTime);
            }

            m_app->AppUpdate(frameTime); //  更新程序

            CameraData cameraData;         //  相机数据
            std::vector<LightData> lights; //  灯光数据

            //  =====================================新增1、：管线式渲染相关=======================================================
            //  【修改】先获取帧缓冲尺寸,驱动管线
            int fbw = 0, fbh = 0;
            glfwGetFramebufferSize(m_window, &fbw, &fbh);
            //  =================================================================================================================

            int width = 0;
            int height = 0;
            //  获取窗口大小
            glfwGetWindowSize(m_window, &width, &height);
            float aspect = static_cast<float>(width) / static_cast<float>(height); //  宽高比

            if (m_currentScene)
            {
                if (auto cameraObject = m_currentScene->GetMainCamera())
                {
                    auto cameraComponent = cameraObject->GetComponent<CameraComponent>();
                    if (cameraComponent)
                    {
                        cameraData.viewMatrix = cameraComponent->GetViewMatrix();
                        cameraData.projectionMatrix = cameraComponent->GetProjectionMatrix(aspect);

                        cameraData.orthoMatrix = glm::ortho(
                            0.0f, static_cast<float>(width),
                            0.0f, static_cast<float>(height));  //  2D正交投影(用于 UI渲染)

                        cameraData.position = cameraComponent->GetEyePosition();
                    }
                }

                lights = m_currentScene->CollectLights();
            }

            //  =====================================新增1、：管线式渲染相关=======================================================
            //  【修改】提交渲染命令:由管线按 Pass 顺序执行(原为 m_renderQueue.Draw)
            if (m_renderPipeline)
            {
                m_renderPipeline->Execute(cameraData, lights, fbw, fbh);
            }
            //  =================================================================================================================

            glfwSwapBuffers(m_window);

            //  清除鼠标输入状态
            m_inputManager.ClearStates();
        }

        // m_app.reset(nullptr);  //  【修改】销毁资源交由 EngDestroy 处理
    }

    void Engine::EngDestroy()
    {
        // if (m_app)
        // {
        //     m_app->AppDestroy();
        //     m_app.reset();

        //     glfwTerminate(); //  释放GLFW所有资源
        //     m_window = nullptr;
        // }

        //  【修改】销毁资源交由 EngDestroy 处理
        if (m_app)
        {
            m_app->AppDestroy();
        }

        //  【关键】场景必须在物理世界还活着时销毁:
        //  场景里的角色/刚体组件析构会调用 removeAction / removeCollisionObject,
        //  拖到进程退出的静态析构阶段(那时物理管理器已先销毁)就会访问已释放的世界 → 段错误
        m_currentScene.reset();

        m_app.reset(nullptr);

        glfwTerminate(); //  释放GLFW所有资源
        m_window = nullptr;
    }

    //  创建程序实例
    void Engine::SetApp(Application *app)
    {
        m_app.reset(app);
    }

    //  获取程序实例
    Application *Engine::GetApp()
    {
        return m_app.get();
    }

    //  获取输入管理器
    InputManager &Engine::GetInputManager()
    {
        return m_inputManager;
    }

    //  获取着色器API
    ShaderAPI &Engine::GetShaderAPI()
    {
        return m_shaderAPI;
    }

    //  获取渲染队列
    RenderQueue &Engine::GetRenderQueue()
    {
        return m_renderQueue;
    }

    //  获取文件系统
    FileSystem &Engine::GetFileSystem()
    {
        return m_fileSystem;
    }

    //  获取纹理管理器
    TextureManager &Engine::GetTextureManager()
    {
        return m_textureManager;
    }

    //  获取物理管理器
    PhysicsManager &Engine::GetPhysicsManager()
    {
        return m_physicsManager;
    }

    //  获取音频管理器
    AudioManager &Engine::GetAudioManager()
    {
        return m_audioManager;
    }

    //  获取字体管理器
    FontManager &Engine::GetFontManager()
    {
        return m_fontManager;
    }

    //  获取UI输入
    UIInputSystem &Engine::GetUIInputSystem()
    {
        return m_uiInputSystem;
    }

    //  设置鼠标回调启用/禁用
    void Engine::SetCursorEnabled(bool enabled)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    //  设置/获取场景
    void Engine::SetScene(const std::shared_ptr<Scene> &scene)
    {
        m_currentScene = scene;
    }
    const std::shared_ptr<Scene> &Engine::GetScene()
    {
        return m_currentScene;
    }

    //  =====================================新增1、：管线式渲染相关=======================================================

    RenderPipeline *Engine::GetRenderPipeline()
    {
        return m_renderPipeline.get();
    }
    SceneRenderer &Engine::GetSceneRenderer()
    {
        return m_sceneRenderer;
    }
    FBOManager &Engine::GetFrameBufferManager()
    {
        return m_frameBuffers;
    }

    //  游戏层自定义渲染 Pass 的便捷接口
    int Engine::AddRenderPass(const RenderPass &pass)
    {
        return m_renderPipeline ? m_renderPipeline->AddPass(pass) : -1;
    }
    void Engine::ClearRenderPasses()
    {
        if (m_renderPipeline)
        {
            m_renderPipeline->ClearPasses();
        }
    }
    const std::shared_ptr<Texture> Engine::GetOutputTexture(const std::string &name, int attachmentIndex) const
    {
        return m_renderPipeline ? m_renderPipeline->GetColorTexture(name, attachmentIndex) : nullptr;
    }

    //  帧缓冲尺寸变化统一入口(供 GLFW 回调调用)
    void Engine::OnFramebufferResize(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return; //  最小化时是 0×0: 非法尺寸, 直接跳过(恢复时会再来)
        }

        m_shaderAPI.SetViewport(0, 0, width, height);
        if (m_renderPipeline)
        {
            m_renderPipeline->Resize(width, height);
        }
    }
    //  =====================================新增2、：物理模拟步长与帧数相关=======================================================
    void Engine::SetPhysicsFixedStep(float seconds)
    {
        m_physicsManager.SetFixedTimeStep(seconds);
    }

    void Engine::SetMaxPhysicsSteps(int steps)
    {
        if (steps > 0)
        {
            m_maxPhysicsSteps = steps;
        }
    }

    // 设置交换间隔
    void Engine::SetSwapInterval(int interval)
    {
        m_swapInterval = interval;
        if (m_window)
        {
            glfwMakeContextCurrent(m_window);
            glfwSwapInterval(m_swapInterval);
        }
    }

    //  =====================================新增1、：管线式渲染相关=======================================================
    //  构建默认渲染管线(EngInit 时调用)
    void Engine::SetupDefaultRenderPipeline()
    {
        if (!m_renderPipeline)
        {
            return;
        }
        m_renderPipeline->ClearPasses();

        // ========================= shadow 相关 Pass =========================

        //  Shadow Pass: 光视角把全场景渲染进深度贴图(所有物体统一用 depth_default)
        RenderPass shadow;
        shadow.name = "Shadow";
        shadow.type = RenderPassType::Geometry; //  复用 Draw3D 遍历同一份命令
        shadow.passTag = PassTag::Shadow;       //  语义标记(非Forward: 不传相机位置/光照)
        shadow.depthTest = 1;
        shadow.shaderName = "depth_default"; //  覆盖着色器: 忽略材质, 全部用深度程序
        shadow.outputFBO = "shadowMap";
        shadow.clearDepthBuffer = true;

        //  深度-only FBO, 固定 2048, 深度纹理由 FBO 内部按 sampler 创建
        shadow.fboSpec.source = FBOSizeSource::Fixed;
        shadow.fboSpec.width = 2048;
        shadow.fboSpec.height = 2048;
        shadow.fboSpec.colorAttachments.clear(); //  无颜色附件
        shadow.fboSpec.depthAttachment.enabled = true;
        shadow.fboSpec.depthAttachment.internalFormat = GL_DEPTH_COMPONENT24;
        shadow.fboSpec.depthAttachment.sampler.minFilter = GL_NEAREST; //  深度采样用 NEAREST
        shadow.fboSpec.depthAttachment.sampler.magFilter = GL_NEAREST;
        //  wrap 保持默认 CLAMP_TO_EDGE ✓(阴影区外不要 REPEAT)

        m_renderPipeline->AddPass(shadow); //  ★ 必须加在 SceneGeometry 之前

        // ========================= ssao 相关 Pass =========================

        //  Depth Prepass: 主相机视角纯深度(给 SSAO 用, 同时顺便提前填深度)
        RenderPass depthPre;
        depthPre.name = "DepthPrepass";
        depthPre.type = RenderPassType::Geometry;
        depthPre.passTag = PassTag::Depth;     // 非 Forward: 不传光照; 相机仍是主相机(只有 Shadow 会切光相机)
        depthPre.shaderName = "depth_default"; // 复用阴影的深度程序
        depthPre.outputFBO = "depthPrepass";
        depthPre.clearDepthBuffer = true;
        depthPre.depthTest = 1;
        depthPre.fboSpec.source = FBOSizeSource::WindowRelative; // 跟随窗口尺寸
        depthPre.fboSpec.widthScale = 1.0f;
        depthPre.fboSpec.heightScale = 1.0f;
        depthPre.fboSpec.colorAttachments.clear();
        depthPre.fboSpec.depthAttachment.enabled = true;
        depthPre.fboSpec.depthAttachment.internalFormat = GL_DEPTH_COMPONENT24;
        depthPre.fboSpec.depthAttachment.sampler.minFilter = GL_NEAREST;
        depthPre.fboSpec.depthAttachment.sampler.magFilter = GL_NEAREST;
        m_renderPipeline->AddPass(depthPre);

        //  SSAO: 全屏, 由深度算遮蔽, 输出 R8 标量
        RenderPass ssao;
        ssao.name = "SSAO";
        ssao.type = RenderPassType::PostProcess;
        ssao.shaderName = "ssao";
        ssao.outputFBO = "ssao";
        ssao.bindCameraUniforms = true; // 注入 uProjection(逆矩阵在 shader 里算)
        ssao.clearColorBuffer = true;
        ssao.clearDepthBuffer = false;
        ssao.clearStencilBuffer = false;
        ssao.depthTest = 0;                                                    // 全屏绘制不参与深度
        ssao.inputBindings.push_back({"depthPrepass", 0, "", "uDepth", true}); // depth=true → 深度附件绑到 uDepth
        ssao.fboSpec.source = FBOSizeSource::WindowRelative;
        ssao.fboSpec.widthScale = 1.0f; // 想提速改成 0.5(半分辨率, 布局/采样全兼容)
        ssao.fboSpec.heightScale = 1.0f;
        ssao.fboSpec.colorAttachments = {{"", false, GL_R8, {GL_LINEAR, GL_LINEAR}}};
        ssao.fboSpec.depthAttachment.enabled = false;
        m_renderPipeline->AddPass(ssao);

        //  SSAO 模糊: 4x4 盒式
        RenderPass ssaoBlur;
        ssaoBlur.name = "SSAOBlur";
        ssaoBlur.type = RenderPassType::PostProcess;
        ssaoBlur.shaderName = "ssao_blur";
        ssaoBlur.outputFBO = "ssaoBlur";
        ssaoBlur.clearColorBuffer = true;
        ssaoBlur.clearDepthBuffer = false;
        ssaoBlur.clearStencilBuffer = false;
        ssaoBlur.depthTest = 0;
        ssaoBlur.inputBindings.push_back({"ssao", 0, "", "uSSAO", false});
        ssaoBlur.fboSpec.source = FBOSizeSource::WindowRelative;
        ssaoBlur.fboSpec.widthScale = 1.0f;
        ssaoBlur.fboSpec.heightScale = 1.0f;
        ssaoBlur.fboSpec.colorAttachments = {{"", false, GL_R8, {GL_LINEAR, GL_LINEAR}}};
        ssaoBlur.fboSpec.depthAttachment.enabled = false;
        m_renderPipeline->AddPass(ssaoBlur);

        // ========================= 场景相关 Pass =========================

        //  几何 Pass: 渲染到离屏 HDR 缓冲(RGBA16F), 供泛光采样; 由合成 Pass 上屏
        RenderPass geometry;
        geometry.name = "SceneGeometry";
        geometry.type = RenderPassType::Geometry;
        geometry.outputFBO = "sceneHDR"; //  ★ 从屏幕改为离屏
        geometry.clearColorBuffer = true;
        geometry.clearDepthBuffer = true;
        geometry.clearColor = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);
        geometry.fboSpec.source = FBOSizeSource::WindowRelative;
        geometry.fboSpec.widthScale = 1.0f;
        geometry.fboSpec.heightScale = 1.0f;
        geometry.fboSpec.colorAttachments = {{"", false, GL_RGBA16F, {GL_LINEAR, GL_LINEAR}}}; //  HDR 半浮点(>1 的亮度能存住)
        geometry.fboSpec.depthAttachment.enabled = true;                                       //  FBO 内自带的深度, 3D 渲染照常深度测试
        m_renderPipeline->AddPass(geometry);

        //  UI Pass:UI 最后绘制到屏幕(不参与后处理)
        RenderPass ui;
        ui.name = "UI";
        ui.type = RenderPassType::UI;
        ui.outputFBO = ""; //  屏幕
        ui.clearColorBuffer = false;
        ui.clearDepthBuffer = false;
        m_renderPipeline->AddPass(ui);
    }
}