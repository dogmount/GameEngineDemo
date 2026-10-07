#ifndef MY_BULLET_H
#define MY_BULLET_H
#pragma once

#include <engine/eng.h>

class Spawner;

class Bullet : public GL_engine::GameObject, public GL_engine::MyContactListener
{
    GAMEOBJECT(Bullet)
public:
    void Update(float deltaTime) override;

    //  接触事件(命中开关板时触发)
    void OnContact(
        GL_engine::CollisionObject *obj,
        const glm::vec3 &pos,
        const glm::vec3 &normal) override;

private:
    void BindSpwaner();

private:
    float m_lifetime = 2.0f;

    bool m_bound = false;

    Spawner *m_startSwitch = nullptr;                  //  "SpawnerStart" 的生成器指针
    GL_engine::CollisionObject *m_startBody = nullptr; //  启动板刚体(用于指针比对)
    GL_engine::CollisionObject *m_stopBody = nullptr;  //  停止板刚体
};

#endif