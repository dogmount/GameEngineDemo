#ifndef PHYSICS_COMPONENT_H
#define PHYSICS_COMPONENT_H
#pragma once

#include "scene/component.h"
#include "physics/rigidBody.h"

namespace GL_engine
{
    class PhysicsComponent : public Component
    {
        COMPONENT(PhysicsComponent)
    public:
        PhysicsComponent() = default;
        PhysicsComponent(const std::shared_ptr<RigidBody> &body);

        //  加载属性
        void LoadProperties(const nlohmann::json &json) override;

        void Init() override;
        void Update(float deltaTime) override;

        //  设置刚体
        void SetRigidBody(const std::shared_ptr<RigidBody> &body);

        //  获取刚体
        const std::shared_ptr<RigidBody> &GetRigidBody();

    private:
        std::shared_ptr<RigidBody> m_rigidBody;
    };
}

#endif