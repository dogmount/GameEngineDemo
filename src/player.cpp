#include "player.h"
#include "bullet.h"

#include <iostream>

#include <GLFW/glfw3.h>

void Player::Init()
{
#if 0
    AddComponent(new GL_engine::CameraComponent());
    SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));
    AddComponent(new GL_engine::PlayerControlComponent());

    auto gun = GL_engine::GameObject::LoadGLTF("models/text/text.gltf");
    gun->SetParent(this);
    gun->SetPosition(glm::vec3(0.75f, -0.5f, -1.0f));
    gun->SetRotation(glm::quat(glm::radians(glm::vec3(30.0f, 20.0f, 50.0f))));
    gun->SetScale(glm::vec3(2.0f, 2.0f, 2.0f));

    if (auto anim = gun->GetComponent<GL_engine::AnimationComponent>())
    {
        if (auto bullet = gun->FindChildByName("bullet_33"))
        {
            bullet->SetActive(false);
        }

        if (auto fire = gun->FindChildByName("BOOM_35"))
        {
            fire->SetActive(false);
        }

        anim->Play("shoot", false);
    }
#endif

    // if (auto bullet = FindChildByName("Fire"))
    // {
    //     bullet->SetActive(false);
    // }

    // if (auto fire = FindChildByName("BOOM_35"))
    // {
    //     fire->SetActive(false);
    // }

    if (auto gun = FindChildByName("Gun"))
    {
        m_animationComponent = gun->GetComponent<GL_engine::AnimationComponent>();
    }

    m_audioComponent = GetComponent<GL_engine::AudioComponent>();
    m_playerControlComponent = GetComponent<GL_engine::PlayerControlComponent>();
}

void Player::Update(float deltaTime)
{
    // 更新冷却计时器
    if (m_shootCooldown > 0.0f)
        m_shootCooldown -= deltaTime;

    GL_engine::GameObject::Update(deltaTime);

    auto &input = GL_engine::Engine::GetInstance().GetInputManager();

    //  射击动作
    if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && m_shootCooldown <= 0.0f)
    {
        m_shootCooldown = 1.0f;

        if (m_animationComponent && !m_animationComponent->IsPlaying())
        {
            m_animationComponent->Play("Fire", false);

            if (m_audioComponent)
            {
                if (m_audioComponent->IsPlaying("Fire"))
                {
                    m_audioComponent->Stop("Fire");
                }
                m_audioComponent->Play("Fire");
            }
        }

        //  添加子弹
        auto bullet = m_scene->CreateObject<Bullet>("bullet");
        auto material = GL_engine::Material::Load("materials/brick.mat");
        auto mesh = GL_engine::Mesh::CreateSphere(0.15f, 32, 32);
        bullet->AddComponent(new GL_engine::MeshComponent(material, mesh));

        glm::vec3 pos = glm::vec3(0.0f);
        if (auto child = FindChildByName("Gun"))
        {
            pos = child->GetWorldPosition();
        }
        bullet->SetPosition(pos + m_rotation * glm::vec3(0.2f, 0.3f, -1.0f));

        auto collider = std::make_shared<GL_engine::SphereCollider>(0.15f);
        auto rigidBody = std::make_shared<GL_engine::RigidBody>(
            GL_engine::BodyType::Dynamic, collider, 10.0f, 0.1f, 0.5f); //  设置弹性系数为0.5

        bullet->AddComponent(new GL_engine::PhysicsComponent(rigidBody));

        glm::vec3 front = m_rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        rigidBody->ApplyImpulse(front * 1000.0f);
    }

    if (input.IsKeyPressed(GLFW_KEY_SPACE))
    {
        // if (m_audioComponent && !m_audioComponent->IsPlaying("jump"))
        // {
        //     m_audioComponent->Play("jump");
        // }
        if (m_audioComponent)
        {
            m_audioComponent->Play("jump");
        }
    }

    bool walking =
        input.IsKeyPressed(GLFW_KEY_W) ||
        input.IsKeyPressed(GLFW_KEY_A) ||
        input.IsKeyPressed(GLFW_KEY_S) ||
        input.IsKeyPressed(GLFW_KEY_D);

    if (walking && m_playerControlComponent && m_playerControlComponent->OnGround())
    {
        if (m_audioComponent && !m_audioComponent->IsPlaying("walk"))
        {
            m_audioComponent->Play("walk", true); //  循环播放
        }
    }
    else
    {
        if (m_audioComponent && m_audioComponent->IsPlaying("walk"))
        {
            m_audioComponent->Stop("walk");
        }
    }
}