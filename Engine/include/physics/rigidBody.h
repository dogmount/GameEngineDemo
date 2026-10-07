#ifndef RIGIDBODY_H
#define RIGIDBODY_H
#pragma once

#include "physics/collider.h"
#include "physics/colliderObject.h"

#include <glm/gtc/quaternion.hpp>
#include <memory>

class btRigidBody;

namespace GL_engine
{
    enum class BodyType
    {
        Static,   //  静态物体（不可移动）
        Dynamic,  //  动态物体
        Kinematic //  运动学物体（质量可视为无限大）
    };

    class RigidBody : public CollisionObject
    {
    public:
        RigidBody(BodyType type, const std::shared_ptr<Collider> &collider, float mass, float friction,float restitution = 0.0f);
        ~RigidBody();

        //  获取刚体
        btRigidBody *GetBody();

        //  添加刚体进入物理模拟
        void SetAddedToWorld(bool added);

        //  判断刚体是否在物理模拟
        bool IsAddedToWorld() const;

        //  获取刚体类型
        BodyType GetType() const;

        //  设置/获取刚体位置
        void SetPosition(const glm::vec3 &pos);
        glm::vec3 GetPosition() const;

        //  设置/获取刚体旋转
        void SetRotation(const glm::quat &rot);
        glm::quat GetRotation() const;

        //  施加冲量
        void ApplyImpulse(const glm::vec3 &impulse);

    private:
        std::unique_ptr<btRigidBody> m_body;
        BodyType m_type = BodyType::Static;
        std::shared_ptr<Collider> m_collider;
        float m_mass = 0.0f;     //  质量
        float m_friction = 0.5f; //  摩擦系数
        float m_restitution = 0.0f; //  弹性系数

        //  是否加入物理模拟
        bool m_addedToWorld = false;
    };
}

#endif