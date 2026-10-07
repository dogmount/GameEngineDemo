#ifndef JUMP_PLATFORM_H
#define JUMP_PLATFORM_H
#pragma once

#include <engine/eng.h>

class JumpPlatform : public GL_engine::GameObject, public GL_engine::MyContactListener
{
    GAMEOBJECT(JumpPlatform)
public:
    void Init() override;

    //  接触事件
    void OnContact(
        GL_engine::CollisionObject *obj,
        const glm::vec3 &pos,
        const glm::vec3 &normal) override;
};

#endif