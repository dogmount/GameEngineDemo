#ifndef COLLIDER_OBJECT_H
#define COLLIDER_OBJECT_H
#pragma once

#include <vector>

#include <glm/glm.hpp>
#include "engine/common.h"

namespace GL_engine
{
    class MyContactListener;

    //  碰撞对象类型
    enum class CollisionObjectType
    {
        RigidBody,
        KinematicCharacterControl
    };

    //  碰撞对象
    class CollisionObject
    {
    public:
        //  获取碰撞对象类型
        CollisionObjectType GetCollisionObjectType();

        //  添加/移除接触监听器
        void AddContactListener(MyContactListener *listener);
        void RemoveContactListener(MyContactListener *listener);

    protected:
        friend class PhysicsManager;

        //  分发接触事件
        void DispatchContactEvent(
            CollisionObject *obj,
            const glm::vec3 &pos,
            const glm::vec3 &normal);

        CollisionObjectType m_collisionObjectType; //  碰撞物体类型

        std::vector<MyContactListener *> m_contactListeners; //  监听器数组
    };

    class MyContactListener
    {
    public:
        //  观察者（任何想要“监听碰撞事件”的类（比如玩家角色、子弹、触发器）都必须实现这个接口
        virtual void OnContact(
            CollisionObject *obj,
            const glm::vec3 &pos,
            const glm::vec3 &normal) = 0;
    };
}

#endif