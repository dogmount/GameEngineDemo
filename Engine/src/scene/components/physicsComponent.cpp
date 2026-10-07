#include "scene/components/physicsComponent.h"
#include "scene/gameObject.h"
#include "engine/Engine.h"

namespace GL_engine
{
    PhysicsComponent::PhysicsComponent(const std::shared_ptr<RigidBody> &body)
        : m_rigidBody(body)
    {
    }

    //  加载属性
    void PhysicsComponent::LoadProperties(const nlohmann::json &json)
    {
        std::shared_ptr<Collider> collider;

        if (json.contains("collider"))
        {
            const auto &colliderObj = json["collider"];
            std::string type = colliderObj.value("type", "");

            if (type == "box")
            {
                glm::vec3 extents(
                    colliderObj.value("x", 1.0f),
                    colliderObj.value("y", 1.0f),
                    colliderObj.value("z", 1.0f));
                collider = std::make_shared<BoxCollider>(extents);
            }
            else if (type == "sphere")
            {
                float radius = colliderObj.value("r", 1.0f);
                collider = std::make_shared<SphereCollider>(radius);
            }
            else if (type == "capsule")
            {
                float radius = colliderObj.value("r", 1.0f);
                float height = colliderObj.value("h", 1.0f);
                collider = std::make_shared<CapsuleCollider>(radius, height);
            }
            else if (type == "prism")
            {
                //  读取三个底面点 + 挤出向量(局部坐标); 缺省给一个沿 +Y 挤出的样本
                auto readVec3 = [&](const char *key, const glm::vec3 &def) -> glm::vec3
                {
                    if (!colliderObj.contains(key) || !colliderObj[key].is_array() || colliderObj[key].size() != 3)
                    {
                        return def;
                    }
                    return glm::vec3(
                        colliderObj[key][0].get<float>(),
                        colliderObj[key][1].get<float>(),
                        colliderObj[key][2].get<float>());
                };

                const glm::vec3 a = readVec3("a", glm::vec3(0.0f, -0.5f, 0.0f));
                const glm::vec3 b = readVec3("b", glm::vec3(1.0f, -0.5f, 0.0f));
                const glm::vec3 c = readVec3("c", glm::vec3(0.0f, -0.5f, 1.0f));
                const glm::vec3 extrude = readVec3("extrude", glm::vec3(0.0f, 1.0f, 0.0f));

                collider = std::make_shared<PrismCollider>(a, b, c, extrude);
            }

            if (!collider)
            {
                return;
            }

            std::shared_ptr<RigidBody> rigidBody;
            if (json.contains("body"))
            {
                const auto &bodyObj = json["body"];

                float mass = bodyObj.value("mass", 0.0f);
                float friction = bodyObj.value("friction", 0.5f);
                float restitution = bodyObj.value("restitution", 0.0f); //  弹性系数
                std::string typeStr = bodyObj.value("type", "static");

                BodyType type = BodyType::Static;
                if (typeStr == "dynamic")
                {
                    type = BodyType::Dynamic;
                }
                else if (typeStr == "kinematic")
                {
                    type = BodyType::Kinematic;
                }

                rigidBody = std::make_shared<RigidBody>(type, collider, mass, friction, restitution);
            }

            if (rigidBody)
            {
                SetRigidBody(rigidBody);
            }
        }
    }

    void PhysicsComponent::Init()
    {
        if (!m_rigidBody)
        {
            return;
        }

        const auto pos = m_owner->GetWorldPosition();
        const auto rot = m_owner->GetWorldRotation();

        m_rigidBody->SetPosition(pos);
        m_rigidBody->SetRotation(rot);

        Engine::GetInstance().GetPhysicsManager().AddRigidBody(m_rigidBody.get());
    }

    void PhysicsComponent::Update(float deltaTime)
    {
        if (!m_rigidBody)
        {
            return;
        }

        if (m_rigidBody->GetType() == BodyType::Dynamic)
        {
            m_owner->SetWorldPosition(m_rigidBody->GetPosition());
            m_owner->SetWorldRotation(m_rigidBody->GetRotation());
        }
    }

    //  设置刚体
    void PhysicsComponent::SetRigidBody(const std::shared_ptr<RigidBody> &body)
    {
        m_rigidBody = body;
    }

    //  获取刚体
    const std::shared_ptr<RigidBody> &PhysicsComponent::GetRigidBody()
    {
        return m_rigidBody;
    }
}