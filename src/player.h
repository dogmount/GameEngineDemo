#ifndef PLAYER_H
#define PLAYER_H
#pragma once

#include <engine/eng.h>

class Player : public GL_engine::GameObject
{
    GAMEOBJECT(Player)
public:
    void Init() override;
    void Update(float deltaTime) override;

private:
    GL_engine::AnimationComponent *m_animationComponent = nullptr;         //  动画组件
    GL_engine::AudioComponent *m_audioComponent = nullptr;                 //  音频组件
    GL_engine::PlayerControlComponent *m_playerControlComponent = nullptr; //  玩家控制器组件

    float m_shootCooldown = 0.0f;
};

#endif