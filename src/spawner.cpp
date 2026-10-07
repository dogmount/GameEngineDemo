#include "spawner.h"

#include <iostream>
#include <random>

//  ---------- 可调参数 ----------
constexpr float kSpawnInterval = 1.0f;   //  生成间隔(秒)
constexpr float kSpawnArea = 20.0f;      //  生成区域半径(以世界原点为中心)
constexpr float kSpawnHeightMin = 10.0f; //  生成高度(从天上掉下来)
constexpr float kSpawnHeightMax = 20.0f;

std::mt19937 &Rng()
{
    static std::mt19937 rng{std::random_device{}()};
    return rng;
}

float RandRange(float lo, float hi)
{
    std::uniform_real_distribution<float> dist(lo, hi);
    return dist(Rng());
}

void Spawner::Update(float deltaTime)
{
    GL_engine::GameObject::Update(deltaTime);

    if (!m_spawning)
    {
        return;
    }

    m_timer -= deltaTime;
    if (m_timer <= 0.0f)
    {
        m_timer = kSpawnInterval;
        SpawnOneRandom();
    }
}

//  开始生成(幂等: 接触事件每帧都会回调, 重复调用直接忽略)
void Spawner::StartSpawning()
{
    if (m_spawning)
    {
        return;
    }
    m_spawning = true;
    m_timer = 0.0f; //  立即生成第一个
    std::cout << "[Spawn] 随机生成已开启\n";
}

//  停止并清除全部生成物
void Spawner::StopSpawningAndClear()
{
    m_spawning = false;

    //  标记销毁 → 场景下一帧回收; PhysicsComponent 析构时刚体会自动从物理世界移除
    for (auto *obj : m_spawned)
    {
        if (obj && obj->IsAlive())
        {
            obj->MarkForDestroy();
        }
    }
    m_spawned.clear();

    std::cout << "[Spawn] 随机生成已关闭, 物体已清除\n";
}

void Spawner::SpawnOneRandom()
{
    if (!m_scene)
    {
        return;
    }

    auto *obj = m_scene->CreateObject("RandomObject");

    //  随机位置(从天上落下) + 随机朝向
    obj->SetPosition(glm::vec3(
        RandRange(-kSpawnArea, kSpawnArea),
        RandRange(kSpawnHeightMin, kSpawnHeightMax),
        RandRange(-50, 50)));
    obj->SetRotationEulerYXZ(glm::vec3(
        RandRange(0.0f, 360.0f), RandRange(0.0f, 360.0f), RandRange(0.0f, 360.0f)));

    //  随机形状: 一半方块, 一半球
    std::shared_ptr<GL_engine::Mesh> mesh;
    std::shared_ptr<GL_engine::Collider> collider;

    if (RandRange(0.0f, 1.0f) < 0.5f)
    {
        const float w = RandRange(0.4f, 1.2f);
        const float h = RandRange(0.4f, 1.2f);
        const float d = RandRange(0.4f, 1.2f);
        mesh = GL_engine::Mesh::CreateBox(w, h, d, 1.0f, 1.0f, 1.0f);
        collider = std::make_shared<GL_engine::BoxCollider>(glm::vec3(w, h, d));
    }
    else
    {
        const float r = RandRange(0.25f, 0.6f);
        mesh = GL_engine::Mesh::CreateSphere(r, 24, 24);
        collider = std::make_shared<GL_engine::SphereCollider>(r);
    }

    //  随机材质(static 缓存, 只加载一次, 所有生成物共享)
    static const std::vector<std::shared_ptr<GL_engine::Material>> materials = {
        GL_engine::Material::Load("materials/brick.mat"),
        GL_engine::Material::Load("materials/wood.mat"),
    };
    std::uniform_int_distribution<size_t> matDist(0, materials.size() - 1);
    auto &material = materials[matDist(Rng())];

    obj->AddComponent(new GL_engine::MeshComponent(material, mesh));

    //  动态刚体: 质量1, 摩擦0.5, 弹性0.3
    auto rigidBody = std::make_shared<GL_engine::RigidBody>(
        GL_engine::BodyType::Dynamic, collider, 1.0f, 0.5f, 0.3f);
    obj->AddComponent(new GL_engine::PhysicsComponent(rigidBody));

    m_spawned.push_back(obj);
}