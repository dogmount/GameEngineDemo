#include "jumpPlatform.h"

void JumpPlatform::Init()
{
    auto physics = GetComponent<GL_engine::PhysicsComponent>();
    if (physics)
    {
        auto rigidBody = physics->GetRigidBody();
        if (rigidBody)
        {
            rigidBody->AddContactListener(this);
        }
    }
}

//  接触事件
void JumpPlatform::OnContact(
    GL_engine::CollisionObject *obj,
    const glm::vec3 &pos,
    const glm::vec3 &normal)
{
    if (obj->GetCollisionObjectType() == GL_engine::CollisionObjectType::KinematicCharacterControl)
    {
        auto control = static_cast<GL_engine::KinematicCharacterControl *>(obj);
        if (control)
        {
            control->Jump(glm::vec3(0.0f, 20.f, 0.0f));
        }
    }
}