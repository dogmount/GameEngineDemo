#include "physics/rigidBody.h"
#include "engine/Engine.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

namespace GL_engine
{
    RigidBody::RigidBody(BodyType type, const std::shared_ptr<Collider> &collider, float mass, float friction, float restitution)
        : m_type(type), m_collider(collider), m_mass(mass), m_friction(friction)
    {
        //  非碰撞体
        if (!collider)
        {
            return;
        }

        m_collisionObjectType = CollisionObjectType::RigidBody;

        btVector3 inertia(0, 0, 0); //  惯性默认值
        if (m_type == BodyType::Dynamic && mass > 0.0f && m_collider->GetShape())
        {
            //  计算惯性
            m_collider->GetShape()->calculateLocalInertia(btScalar(mass), inertia);
        }

        //  Bullet变换初始化
        btTransform transform;
        transform.setIdentity(); //  设置位置于世界原点

        //  初始移动状态
        btDefaultMotionState *motionState = new btDefaultMotionState(transform);

        //  刚体构造信息
        btRigidBody::btRigidBodyConstructionInfo info(
            (m_type == BodyType::Dynamic) ? btScalar(mass) : btScalar(0),
            motionState, m_collider->GetShape(), inertia);

        m_body = std::make_unique<btRigidBody>(info);
        m_body->setFriction(friction);
        m_body->setRestitution(restitution); //  设置弹性系数
        m_body->setUserPointer(this);

        if (m_type == BodyType::Kinematic)
        {
            //  设置运动学物体标志，告知Bullet该物体质量为0但可以施加冲量
            m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            m_body->setActivationState(DISABLE_DEACTIVATION); //  禁用 Bullet 的自动休眠机制
        }
    }

    RigidBody::~RigidBody()
    {
        if (m_addedToWorld)
        {
            Engine::GetInstance().GetPhysicsManager().RemoveRigidBody(this);
        }

        //  btRigidBody 不拥有 motionState(其析构不删), 必须手动释放
        if (m_body && m_body->getMotionState())
        {
            delete m_body->getMotionState();
        }
    }

    //  获取刚体
    btRigidBody *RigidBody::GetBody()
    {
        return m_body.get();
    }

    //  添加刚体进入物理模拟
    void RigidBody::SetAddedToWorld(bool added)
    {
        m_addedToWorld = added;
    }

    //  判断刚体是否在物理模拟
    bool RigidBody::IsAddedToWorld() const
    {
        return m_addedToWorld;
    }

    //  获取刚体类型
    BodyType RigidBody::GetType() const
    {
        return m_type;
    }

    //  设置/获取刚体位置
    void RigidBody::SetPosition(const glm::vec3 &pos)
    {
        if (!m_body)
        {
            return;
        }

        auto &tr = m_body->getWorldTransform();
        tr.setOrigin(btVector3(btScalar(pos.x), btScalar(pos.y), btScalar(pos.z))); //  设置刚体几何中心世界空间位置
        if (m_body->getMotionState())
        {
            m_body->getMotionState()->setWorldTransform(tr);
        }
        m_body->setWorldTransform(tr);
    }
    glm::vec3 RigidBody::GetPosition() const
    {
        if (!m_body)
        {
            return glm::vec3(0.0f);
        }
        const auto &pos = m_body->getWorldTransform().getOrigin();
        return glm::vec3(pos.x(), pos.y(), pos.z());
    }

    //  设置/获取刚体旋转
    void RigidBody::SetRotation(const glm::quat &rot)
    {
        if (!m_body)
        {
            return;
        }

        auto &tr = m_body->getWorldTransform();
        tr.setRotation(btQuaternion(btScalar(rot.x), btScalar(rot.y), btScalar(rot.z), btScalar(rot.w))); //  设置刚体旋转
        if (m_body->getMotionState())
        {
            m_body->getMotionState()->setWorldTransform(tr);
        }
        m_body->setWorldTransform(tr);
    }
    glm::quat RigidBody::GetRotation() const
    {
        if (!m_body)
        {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }
        const auto &rot = m_body->getWorldTransform().getRotation();
        return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
    }

    //  施加冲量
    void RigidBody::ApplyImpulse(const glm::vec3 &impulse)
    {
        if (!m_body)
        {
            return;
        }
        m_body->applyCentralImpulse(btVector3(
            btScalar(impulse.x), btScalar(impulse.y), btScalar(impulse.z)));
    }
}