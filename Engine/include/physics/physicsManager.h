#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H
#pragma once

#include <memory>

class btBroadphaseInterface;               //  粗检测阶段接口
class btDefaultCollisionConfiguration;     //  碰撞配置类
class btCollisionDispatcher;               //  碰撞分发器
class btSequentialImpulseConstraintSolver; //  顺序冲量约束求解器
class btDiscreteDynamicsWorld;             //  离散动力学世界

namespace GL_engine
{
    static constexpr float kDefaultFixedTimeStep = 1.0f / 60.0f; // 默认 60Hz

    class RigidBody;

    class PhysicsManager
    {
    public:
        PhysicsManager();
        ~PhysicsManager();

        void Init();

        //  新增：设置/获取物理模拟时间步长
        void SetFixedTimeStep(float seconds);
        float GetFixedTimeStep() const;

        //  新增: 推进"恰好一个固定步"(内部不累加), 每步结束后分发碰撞事件
        void Step(float fixedDeltaTime);

        //  添加/移除刚体
        void AddRigidBody(RigidBody *body);
        void RemoveRigidBody(RigidBody *body);

        //  获取物理模拟世界
        btDiscreteDynamicsWorld *GetWorld();

    private:
        //  新增： 从 dispatcher 收集碰撞流形并分发接触事件(Step/Update 共用)
        void DispatchContactEvents();

        std::unique_ptr<btBroadphaseInterface> m_broadphase;
        std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
        std::unique_ptr<btCollisionDispatcher> m_dispatcher;
        std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
        std::unique_ptr<btDiscreteDynamicsWorld> m_world;

        float m_fixedTimeStep = kDefaultFixedTimeStep; //  新增：固定步长(可由引擎配置)
    };
}

#endif