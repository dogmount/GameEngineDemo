#ifndef KINEMATIC_CHARACTER_CONTROL_H
#define KINEMATIC_CHARACTER_CONTROL_H
#pragma once

#include "physics/colliderObject.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

class btPairCachingGhostObject;
class btKinematicCharacterController;
class btCapsuleShape;

namespace GL_engine
{
    class KinematicCharacterControl : public CollisionObject
    {
    public:
        KinematicCharacterControl(float radius, float height, const glm::vec3 &position); //  基于胶囊碰撞体，需要半径、高度
        ~KinematicCharacterControl();

        //  获取位置/旋转
        glm::vec3 GetPosition() const;
        glm::quat GetRotation() const;

        //  玩家行为函数
        void Walk(const glm::vec3 &direction);
        void Jump(const glm::vec3 &direction);
        void Push(float pushSpeed);

        //  获取玩家与地面接触状态
        bool OnGround() const;

    private:
        float m_radius = 1.2f;
        float m_height = 0.4f;

        std::unique_ptr<btCapsuleShape> m_capsule;
        std::unique_ptr<btPairCachingGhostObject> m_ghost;
        std::unique_ptr<btKinematicCharacterController> m_controller;
    };
}

#endif