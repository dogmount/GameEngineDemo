#include <iostream>

#include "game.h"
#include "player.h"
#include "bullet.h"
#include "jumpPlatform.h"
#include "spawner.h"

#include <GLFW/glfw3.h>

void Game::RegisterTypes()
{
    Player::Register();
    Bullet::Register();
    JumpPlatform::Register();
    Spawner::Register(); //  新增
}

//  初始化
bool Game::AppInit()
{
    auto &engine = GL_engine::Engine::GetInstance();

    auto scene = GL_engine::Scene::Load("scenes/scene1.sc");
    m_scene = scene;

    engine.SetScene(m_scene);

    engine.SetPhysicsFixedStep(1.0f / 180.0f);

    m_3DRoot = m_scene->FindObjectByName("3DRoot");
    if (m_3DRoot)
    {
        m_3DRoot->SetActive(false);
    }

    auto canvasComponent = engine.GetUIInputSystem().GetCanvas();
    if (!canvasComponent)
    {
        return false;
    }
    canvasComponent->SetActive(true);
    engine.SetCursorEnabled(true);
    engine.GetUIInputSystem().SetActive(true);

    if (auto button = canvasComponent->GetOwner()->FindChildByName("PlayButton"))
    {
        if (auto component = button->GetComponent<GL_engine::ButtonComponent>())
        {
            //  按下PlayButton隐藏画布与鼠标，启动3D渲染
            component->onClick = [this]()
            {
                auto &engine = GL_engine::Engine::GetInstance();
                engine.GetUIInputSystem().GetCanvas()->SetActive(false);
                engine.SetCursorEnabled(false);

                if (m_3DRoot)
                {
                    m_3DRoot->SetActive(true);
                }
            };
        }
    }

    if (auto button = canvasComponent->GetOwner()->FindChildByName("QuitButton"))
    {
        if (auto component = button->GetComponent<GL_engine::ButtonComponent>())
        {
            //  按下PlayButton隐藏画布与鼠标，启动3D渲染
            component->onClick = [this]()
            {
                SetAppToClose(true);
            };
        }
    }
    m_sun = m_scene->FindObjectByName("Sun");

#if 0
    //  2D测试
    m_scene = std::make_shared<GL_engine::Scene>();
    GL_engine::Engine::GetInstance().SetScene(m_scene);

    auto sprite = m_scene->CreateObject("Sprite");
    auto spriteComponent = new GL_engine::SpriteComponent();
    auto texture = GL_engine::Texture::Load("textures/container.jpg");
    spriteComponent->SetTexture(texture);

    sprite->AddComponent(spriteComponent);
    sprite->SetPosition2D(glm::vec2(600.f, 900.f));

    spriteComponent->SetSize(glm::vec2(200.f, 100.f));
    spriteComponent->SetUpperRightUV(glm::vec2(2.0f, 1.0f));
    sprite->SetRotation2D(glm::radians(45.0f));

    auto camera = m_scene->CreateObject("camera");
    auto cameraComponent = new GL_engine::CameraComponent();
    camera->AddComponent(cameraComponent);
    m_scene->SetMainCamera(camera);

    //  ui测试
    auto canvas = m_scene->CreateObject("Canvas");
    auto canvasComponent = new GL_engine::CanvasComponent();
    canvas->AddComponent(canvasComponent);

    //  按钮UI
    auto &uiInput = GL_engine::Engine::GetInstance().GetUIInputSystem();
    uiInput.SetActive(true);
    uiInput.SetCanvas(canvasComponent);

    auto button = m_scene->CreateObject("button", canvas);
    button->SetPosition2D(glm::vec2(300.0f, 300.0f));
    auto buttonComponent = new GL_engine::ButtonComponent();
    buttonComponent->SetRect(glm::vec2(150.0f, 50.0f));
    buttonComponent->SetColor(glm::vec4(1.0f, 0.8f, 0.8f, 1.0f));
    button->AddComponent(buttonComponent);

    auto text = m_scene->CreateObject("Text", canvas);
    text->SetPosition2D(glm::vec2(300.0f, 300.0f));
    auto textComponent = new GL_engine::TextComponent();
    text->AddComponent(textComponent);
    textComponent->SetText("!Some Text");
    textComponent->SetFont("fonts/BlockZone.ttf", 24);
    textComponent->SetColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    // textComponent->SetFont("fonts/default.ttf", 24);
#endif

    //  ========================= 天空盒 =========================
    {
        auto &shaderAPI = engine.GetShaderAPI();

        //  1) 注册天空盒着色器
        auto skyboxShader = shaderAPI.LoadShaderFromPath(
            "skybox",
            "shaders/skybox/vertex.glsl",
            "shaders/skybox/fragment.glsl");

        //  2) 加载立方体贴图(文件夹里放 right/left/top/bottom/front/back 六张图)
        auto skyboxTexture = GL_engine::Texture::LoadCubeTexture("textures/skybox");

        //  3) 立方体网格(尺寸随意, 顶点位置即采样方向; 相机始终在盒内)
        auto skyboxMesh = GL_engine::Mesh::CreateBox(2.0f, 2.0f, 2.0f, 1.0f, 1.0f, 1.0f);

        if (skyboxShader && skyboxTexture && skyboxMesh)
        {
            GL_engine::RenderPass skyboxPass;
            skyboxPass.name = "Skybox";
            skyboxPass.type = GL_engine::RenderPassType::Custom;
            skyboxPass.depthTest = 1;            // 只填背景像素, 需要深度测试
            skyboxPass.clearColorBuffer = false; // 叠在场景后面, 不清理不写深度区
            skyboxPass.clearDepthBuffer = false;
            skyboxPass.clearStencilBuffer = false;

            skyboxPass.outputFBO = "sceneHDR"; //  从屏幕改为 sceneHDR
            skyboxPass.fboSpec.source = GL_engine::FBOSizeSource::WindowRelative;
            skyboxPass.fboSpec.widthScale = 1.0f;
            skyboxPass.fboSpec.heightScale = 1.0f;
            skyboxPass.fboSpec.colorAttachments = {{"", false, GL_RGBA16F, {GL_LINEAR, GL_LINEAR}}};
            skyboxPass.fboSpec.depthAttachment.enabled = true;

            skyboxPass.custom =
                [skyboxShader, skyboxTexture, skyboxMesh](
                    GL_engine::ShaderAPI &,
                    GL_engine::FBOManager &,
                    GL_engine::SceneRenderer &,
                    const GL_engine::CameraData &camera,
                    const std::vector<GL_engine::LightData> &)
            {
                skyboxShader->ShaderBind();

                //  视图矩阵去掉平移 → 天空盒永远以相机为中心
                glm::mat4 view = camera.viewMatrix;
                view[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

                skyboxShader->SetUniform("uView", view);
                skyboxShader->SetUniform("uProjection", camera.projectionMatrix);
                skyboxShader->SetTexture("skybox", skyboxTexture.get());

                skyboxMesh->MeshBind();
                skyboxMesh->MeshDraw();
                skyboxMesh->MeshUnbind();
            };

            //  插到 UI 之前: 管线顺序变成 Shadow → SceneGeometry → Skybox → UI
            auto *pipeline = engine.GetRenderPipeline();
            pipeline->InsertPass(static_cast<int>(pipeline->GetPassCount()) - 1, skyboxPass);
        }
    }

    //  ========================= 泛光(Bloom) =========================
    {
        auto &shaderAPI = engine.GetShaderAPI();
        shaderAPI.LoadShaderFromPath("bloom_bright", "shaders/bloom/vertex.glsl", "shaders/bloom/bright.glsl");
        shaderAPI.LoadShaderFromPath("bloom_blur", "shaders/bloom/vertex.glsl", "shaders/bloom/blur.glsl");
        shaderAPI.LoadShaderFromPath("bloom_composite", "shaders/bloom/vertex.glsl", "shaders/bloom/composite.glsl");

        auto *pipeline = engine.GetRenderPipeline();

        //  ---------- 1) 亮部提取: sceneHDR → bloomBright(半分辨率) ----------
        GL_engine::RenderPass bright;
        bright.name = "BloomBright";
        bright.type = GL_engine::RenderPassType::PostProcess;
        bright.shaderName = "bloom_bright";
        bright.outputFBO = "bloomBright";
        bright.clearColorBuffer = true;
        bright.clearDepthBuffer = false;
        bright.clearStencilBuffer = false;
        bright.depthTest = 0;
        bright.inputBindings.push_back({"sceneHDR", 0, "", "uScene", false});
        bright.uniforms.emplace_back("uThreshold", 1.0f);
        bright.uniforms.emplace_back("uKnee", 0.6f);
        bright.fboSpec.source = GL_engine::FBOSizeSource::WindowRelative;
        bright.fboSpec.widthScale = 0.5f; //  半分辨率: 泛光是低频信号, 省一半开销
        bright.fboSpec.heightScale = 0.5f;
        bright.fboSpec.colorAttachments = {{"", false, GL_RGBA16F, {GL_LINEAR, GL_LINEAR}}};
        bright.fboSpec.depthAttachment.enabled = false;
        pipeline->InsertPass(static_cast<int>(pipeline->GetPassCount()) - 1, bright);

        //  ---------- 2) 横向模糊: bloomBright → bloomBlurH ----------
        GL_engine::RenderPass blurH;
        blurH.name = "BloomBlurH";
        blurH.type = GL_engine::RenderPassType::PostProcess;
        blurH.shaderName = "bloom_blur";
        blurH.outputFBO = "bloomBlurH";
        blurH.clearColorBuffer = true;
        blurH.clearDepthBuffer = false;
        blurH.clearStencilBuffer = false;
        blurH.depthTest = 0;
        blurH.inputBindings.push_back({"bloomBright", 0, "", "uSource", false});
        blurH.uniforms.emplace_back("uDirection", glm::vec2(1.0f, 0.0f));
        blurH.fboSpec.source = GL_engine::FBOSizeSource::WindowRelative;
        blurH.fboSpec.widthScale = 0.5f;
        blurH.fboSpec.heightScale = 0.5f;
        blurH.fboSpec.colorAttachments = {{"", false, GL_RGBA16F, {GL_LINEAR, GL_LINEAR}}};
        blurH.fboSpec.depthAttachment.enabled = false;
        pipeline->InsertPass(static_cast<int>(pipeline->GetPassCount()) - 1, blurH);

        //  ---------- 3) 纵向模糊: bloomBlurH → bloomFinal ----------
        GL_engine::RenderPass blurV;
        blurV.name = "BloomBlurV";
        blurV.type = GL_engine::RenderPassType::PostProcess;
        blurV.shaderName = "bloom_blur";
        blurV.outputFBO = "bloomFinal";
        blurV.clearColorBuffer = true;
        blurV.clearDepthBuffer = false;
        blurV.clearStencilBuffer = false;
        blurV.depthTest = 0;
        blurV.inputBindings.push_back({"bloomBlurH", 0, "", "uSource", false});
        blurV.uniforms.emplace_back("uDirection", glm::vec2(0.0f, 1.0f));
        blurV.fboSpec.source = GL_engine::FBOSizeSource::WindowRelative;
        blurV.fboSpec.widthScale = 0.5f;
        blurV.fboSpec.heightScale = 0.5f;
        blurV.fboSpec.colorAttachments = {{"", false, GL_RGBA16F, {GL_LINEAR, GL_LINEAR}}};
        blurV.fboSpec.depthAttachment.enabled = false;
        pipeline->InsertPass(static_cast<int>(pipeline->GetPassCount()) - 1, blurV);

        //  ---------- 4) 合成: sceneHDR + bloomFinal → 屏幕 ----------
        GL_engine::RenderPass composite;
        composite.name = "BloomComposite";
        composite.type = GL_engine::RenderPassType::PostProcess;
        composite.shaderName = "bloom_composite";
        composite.outputFBO = "";           //  空 = 屏幕
        composite.clearColorBuffer = false; //  全屏覆盖, 无需清屏
        composite.clearDepthBuffer = false;
        composite.clearStencilBuffer = false;
        composite.depthTest = 0;
        composite.inputBindings.push_back({"sceneHDR", 0, "", "uScene", false});
        composite.inputBindings.push_back({"bloomFinal", 0, "", "uBloom", false});
        composite.uniforms.emplace_back("uBloomIntensity", 1.2f);
        pipeline->InsertPass(static_cast<int>(pipeline->GetPassCount()) - 1, composite);
    }

    //  场景：

    return true;
}

//  每帧更新
void Game::AppUpdate(float deltaTime)
{
    m_scene->Update(deltaTime);

    auto &engine = GL_engine::Engine::GetInstance();
    if (engine.GetInputManager().IsKeyPressed(GLFW_KEY_ESCAPE))
    {
        if (m_3DRoot && m_3DRoot->IsActive())
        {
            engine.GetUIInputSystem().GetCanvas()->SetActive(true);
            engine.SetCursorEnabled(true);
            m_3DRoot->SetActive(false);
        }
    }

    //  意图: 绕场景旋转的太阳 —— 位置每帧被 CollectLights 读取, 光照方向实时生效
    // if (m_sun)
    // {
    //     static float sunAngle = 0.6f;  //  起始角: 避开"垂直正上方"(否则高光会钉在脚下)
    //     sunAngle += deltaTime * 0.35f; //  角速度 rad/s, 调大小控制日夜快慢

    //     const float radius = 20.0f;
    //     m_sun->SetPosition(glm::vec3(
    //         glm::cos(sunAngle) * radius,
    //         20.0f, //  保持高度, 别让它落到地平线以下
    //         glm::sin(sunAngle) * radius));
    // }
}

//  释放资源
void Game::AppDestroy()
{
}