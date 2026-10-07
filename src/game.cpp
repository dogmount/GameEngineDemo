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