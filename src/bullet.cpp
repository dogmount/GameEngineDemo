#include "bullet.h"
#include "spawner.h"

#include <iostream>

void Bullet::BindSpwaner()
{
    m_bound = true;

    //  1) 给自己注册接触监听(子弹被 CreateObject 创建, 不在 Init 流程里, 所以首帧延迟绑定)
    if (auto physics = GetComponent<GL_engine::PhysicsComponent>())
    {
        if (auto body = physics->GetRigidBody())
        {
            body->AddContactListener(this);
        }
    }

    //  2) 按名字解析场景里的两块开关板
    auto *scene = GetScene();
    if (!scene)
    {
        return;
    }

    auto bodyOf = [](GL_engine::GameObject *go) -> GL_engine::CollisionObject *
    {
        if (!go)
        {
            return nullptr;
        }
        auto physics = go->GetComponent<GL_engine::PhysicsComponent>();
        return physics ? physics->GetRigidBody().get() : nullptr;
    };

    auto *startGo = scene->FindObjectByName("SpawnerStart");
    auto *stopGo = scene->FindObjectByName("SpawnerStop");

    m_startSwitch = dynamic_cast<Spawner *>(startGo); //  类型不对会是 nullptr
    m_startBody = bodyOf(startGo);
    m_stopBody = bodyOf(stopGo);

    if (!m_startBody || !m_stopBody || !m_startSwitch)
    {
        std::cerr << "[Bullet] SpawnerStart/SpawnerStop 解析失败: 检查场景中对象名称、"
                     "SpawnerStart 的 type 是否为 Spawner、两者是否有 PhysicsComponent\n";
    }
}

void Bullet::Update(float deltaTime)
{
    GL_engine::GameObject::Update(deltaTime);
    m_lifetime -= deltaTime;

    if (!m_bound)
    {
        BindSpwaner(); //  首帧: 此时组件与场景都已就绪
    }

    if (m_lifetime <= 0.0f)
    {
        MarkForDestroy();
    }
}

void Bullet::OnContact(
    GL_engine::CollisionObject *obj,
    const glm::vec3 &pos,
    const glm::vec3 &normal)
{
    if (obj == m_startBody) //  命中启动板 → 开始随机生成
    {
        if (m_startSwitch)
        {
            m_startSwitch->StartSpawning();
        }
        MarkForDestroy(); //  命中即消失(接触每帧都会回调, 避免反复触发/反弹再碰)
    }
    else if (obj == m_stopBody) //  命中停止板 → 停止并清除
    {
        if (m_startSwitch)
        {
            m_startSwitch->StopSpawningAndClear();
        }
        MarkForDestroy();
    }
}