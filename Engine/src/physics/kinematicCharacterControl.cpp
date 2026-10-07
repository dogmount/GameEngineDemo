#include "physics/kinematicCharacterControl.h"
#include "engine/Engine.h"

#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

namespace GL_engine
{
    //  基于胶囊碰撞体，需要半径、高度
    KinematicCharacterControl::KinematicCharacterControl(float radius, float height, const glm::vec3 &position)
        : m_radius(radius), m_height(height)
    {
        m_collisionObjectType = CollisionObjectType::KinematicCharacterControl;

        auto world = Engine::GetInstance().GetPhysicsManager().GetWorld();

        m_capsule = std::make_unique<btCapsuleShape>(m_radius, m_height); //  创建胶囊构造体

        m_ghost = std::make_unique<btPairCachingGhostObject>(); //  创建幽灵对象
        /*
        幽灵对象是一种特殊的碰撞物体，它不产生物理响应（不会被力推动），但会持续报告与其他物体的碰撞重叠。
        */

        btTransform start;
        start.setIdentity();
        start.setOrigin(btVector3(position.x, position.y, position.z));

        //  设置位置/碰撞形状/碰撞标志
        m_ghost->setWorldTransform(start);
        m_ghost->setCollisionShape(m_capsule.get());

        //  添加角色对象标志，改变碰撞求解器对该物体的处理优先级
        m_ghost->setCollisionFlags(m_ghost->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT);

        //  设置用户指针
        m_ghost->setUserPointer(this);

        const btScalar stepHeight = 0.35f; //  可攀爬台阶高度
        m_controller = std::make_unique<btKinematicCharacterController>(m_ghost.get(), m_capsule.get(), stepHeight);

        m_controller->setMaxSlope(btRadians(50.0f)); //  角色可攀爬最大斜坡角度
        m_controller->setGravity(world->getGravity());

        world->addCollisionObject(
            m_ghost.get(),
            btBroadphaseProxy::CharacterFilter,
            btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
        world->addAction(m_controller.get());
    }

    KinematicCharacterControl::~KinematicCharacterControl()
    {
        auto world = Engine::GetInstance().GetPhysicsManager().GetWorld();
        if (m_controller)
        {
            world->removeAction(m_controller.get());
        }
        if (m_ghost)
        {
            world->removeCollisionObject(m_ghost.get());
        }
    }

    //  获取位置/旋转
    glm::vec3 KinematicCharacterControl::GetPosition() const
    {
        const auto &pos = m_ghost->getWorldTransform().getOrigin();
        const glm::vec3 offset(0.0f, m_height + 0.5f + m_radius, 0.0f); //  为相机设置偏移量
        return glm::vec3(pos.x(), pos.y(), pos.z()) + offset;
    }
    glm::quat KinematicCharacterControl::GetRotation() const
    {
        const auto &rot = m_ghost->getWorldTransform().getRotation();
        return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
    }

    //  玩家行为函数
    void KinematicCharacterControl::Walk(const glm::vec3 &direction)
    {
        m_controller->setWalkDirection(btVector3(
            btScalar(direction.x),
            btScalar(direction.y),
            btScalar(direction.z)));
    }
    void KinematicCharacterControl::Jump(const glm::vec3 &direction)
    {
        if (m_controller->onGround())
        {
            m_controller->jump(btVector3(
                btScalar(direction.x),
                btScalar(direction.y),
                btScalar(direction.z)));
        }
    }

    void KinematicCharacterControl::Push(float pushSpeed)
    {
        if (!m_ghost)
        {
            return;
        }

        const btVector3 myPos = m_ghost->getWorldTransform().getOrigin();
        const int count = m_ghost->getNumOverlappingObjects(); //  该列表每物理步自动刷新

        for (int i = 0; i < count; ++i)
        {
            btCollisionObject *other = m_ghost->getOverlappingObject(i);

            auto *rb = btRigidBody::upcast(other);
            if (!rb || rb->isStaticOrKinematicObject())
            {
                continue; //  只推"动态"刚体
            }

            //  水平推挤方向: 从角色指向物体(不推上下, 避免把箱子按进地里)
            btVector3 dir = other->getWorldTransform().getOrigin() - myPos;
            dir.setY(0.0f);
            if (dir.length2() < 1e-6f)
            {
                continue;
            }
            dir.normalize();

            //  速度逼近式: J = m·Δv, Δv = pushSpeed - 当前沿方向速度
            //  意图: 箱子跟着角色走, 但不会越推越快/被打飞
            //  applyCentralImpulse 会自动唤醒休眠刚体(不需要手动 activate)
            const btScalar vAlong = rb->getLinearVelocity().dot(dir);
            if (vAlong < pushSpeed)
            {
                // rb->activate(true); //  内部: setActivationState(ACTIVE_TAG) + 重置休眠计时器
                rb->applyCentralImpulse(dir * rb->getMass() * (pushSpeed - vAlong));
            }
        }
    }

    //  获取玩家、地面接触状态
    bool KinematicCharacterControl::OnGround() const
    {
        return m_controller->onGround();
    }
}