#include "physics/physicsManager.h"
#include "physics/rigidBody.h"
#include "physics/colliderObject.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

namespace GL_engine
{
    PhysicsManager::PhysicsManager()
    {
    }

    PhysicsManager::~PhysicsManager()
    {
    }

    void PhysicsManager::Init()
    {
        //  顺序实例化bullet子系统
        m_broadphase = std::make_unique<btDbvtBroadphase>();
        m_collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
        m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfig.get());
        m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        m_world = std::make_unique<btDiscreteDynamicsWorld>(m_dispatcher.get(), m_broadphase.get(), m_solver.get(), m_collisionConfig.get());

        //  设置角色幽灵对象的堆叠对（未设置时幽灵对象无正常碰撞检测
        m_world->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback);

        m_world->setGravity(btVector3(0, -9.81f, 0));
    }

    //  新增：设置/获取物理模拟时间步长
    void PhysicsManager::SetFixedTimeStep(float seconds)
    {
        if (seconds > 0)
        {
            m_fixedTimeStep = seconds;
        }
    }
    float PhysicsManager::GetFixedTimeStep() const
    {
        return m_fixedTimeStep;
    }

    //  新增: 推进"恰好一个固定步"(内部不累加), 每步结束后分发碰撞事件
    void PhysicsManager::Step(float fixedDeltaTime)
    {
        if (!m_world)
        {
            return;
        }

        //  timeStep 恰好等于 fixedTimeStep → Bullet 内部只执行 1 步, 不再有自己的累加器
        m_world->stepSimulation(fixedDeltaTime, 1, fixedDeltaTime);

        DispatchContactEvents();
    }

    /*
    ***旧物理管理器更新函数
    由于物理模拟步长与帧数相关, 可能会出现不稳定的情况, 因此弃用
    现在使用 Step(fixedDeltaTime) 显式推进"恰好一个固定步"(内部不累加), 每步结束后分发碰撞事件

    void PhysicsManager::Update(float deltaTime)
    {
        const btScalar fixedTimeStep = 1.0 / 60.0f;                     //  60Hz固定步长
        const int maxSubsteps = 4;                                      //  限制最大模拟步长
        m_world->stepSimulation(deltaTime, maxSubsteps, fixedTimeStep); //  模拟步长

        DispatchContactEvents();
    }
    */

    //  添加/移除刚体
    void PhysicsManager::AddRigidBody(RigidBody *body)
    {
        if (!body || !m_world)
        {
            return;
        }

        // if (auto rigidBody = body->GetBody())
        // {
        //     /*
        //         参数 1（rigidBody）：要添加的刚体指针。
        //         参数 2（Collision Group / 碰撞组）：StaticFilter —— 设定这个刚体属于 “静态组”。
        //         参数 3（Collision Mask / 碰撞掩码）：AllFilter —— 设定这个刚体可以与所有组发生碰撞
        //     */
        //     m_world->addRigidBody(rigidBody, btBroadphaseProxy::StaticFilter, btBroadphaseProxy::AllFilter);
        //     body->SetAddedToWorld(true);
        // }

        //  按照类型设置刚体过滤
        if (auto rigidBody = body->GetBody())
        {
            int group = btBroadphaseProxy::DefaultFilter; //  默认给动态物体
            switch (body->GetType())
            {
            case BodyType::Static:
                group = btBroadphaseProxy::StaticFilter;
                break;
            case BodyType::Kinematic:
                group = btBroadphaseProxy::KinematicFilter;
                break;
            case BodyType::Dynamic:
            default:
                group = btBroadphaseProxy::DefaultFilter;
                break;
            }

            m_world->addRigidBody(rigidBody, group, btBroadphaseProxy::AllFilter);
            body->SetAddedToWorld(true);
        }
    }
    void PhysicsManager::RemoveRigidBody(RigidBody *body)
    {
        if (!body || !m_world)
        {
            return;
        }

        if (auto rigidBody = body->GetBody())
        {
            m_world->removeRigidBody(rigidBody);
            body->SetAddedToWorld(false);
        }
    }

    //  获取物理模拟世界
    btDiscreteDynamicsWorld *PhysicsManager::GetWorld()
    {
        return m_world.get();
    }

    //  新增： 从 dispatcher 收集碰撞流形并分发接触事件(Step/Update 共用)
    void PhysicsManager::DispatchContactEvents()
    {
        //  过程碰撞
        auto dispatcher = m_world->getDispatcher();
        const auto numManifolds = dispatcher->getNumManifolds(); //  获取碰撞流行的总数量(A撞B 则为1; A撞B和C 则为2)
        for (int i = 0; i < numManifolds; ++i)
        {
            auto manifold = dispatcher->getManifoldByIndexInternal(i);
            if (!manifold)
            {
                continue;
            }

            //  获取碰撞物体的指针
            auto bodyA = reinterpret_cast<CollisionObject *>(manifold->getBody0()->getUserPointer());
            auto bodyB = reinterpret_cast<CollisionObject *>(manifold->getBody1()->getUserPointer());

            if (!bodyA || !bodyB)
            {
                continue;
            }

            //  获取碰撞接触点个数
            const auto numContacts = manifold->getNumContacts();
            for (int j = 0; j < numContacts; ++j)
            {
                const auto &point = manifold->getContactPoint(j); //  获取碰撞点
                const glm::vec3 pos(
                    point.m_positionWorldOnB.x(),
                    point.m_positionWorldOnB.y(),
                    point.m_positionWorldOnB.z());

                const glm::vec3 normal(
                    point.m_normalWorldOnB.x(),
                    point.m_normalWorldOnB.y(),
                    point.m_normalWorldOnB.z());

                //  分发碰撞事件
                bodyA->DispatchContactEvent(bodyB, pos, normal);
                bodyB->DispatchContactEvent(bodyA, pos, normal);
            }
        }
    }
}