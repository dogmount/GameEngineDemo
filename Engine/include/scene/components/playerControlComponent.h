#ifndef PLAYER_CONTROL_COMPONENT_H
#define PLAYER_CONTROL_COMPONENT_H
#pragma once

#include "scene/component.h"
#include "physics/kinematicCharacterControl.h"
#include <memory>

namespace GL_engine
{
    class PlayerControlComponent : public Component
    {
        COMPONENT(PlayerControlComponent)
    public:
        void Init() override;
        void Update(float deltaTime) override;

        //  获取玩家与地面接触状态
        bool OnGround() const;

    private:
        //  第一次镜头
        // bool isFirst = true;

        float m_sensitivity = 0.05f; //  鼠标灵敏度
        float m_moveSpeed = 5.0f;    //  移动速度

        float m_xRot = 0.0f; //  x旋转角度
        float m_yRot = 0.0f; //  y旋转角度

        std::unique_ptr<KinematicCharacterControl> m_kinematicControl; //  玩家物理控制器
    };
}

#endif