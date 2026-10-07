#ifndef MY_GAME_H
#define MY_GAME_H
#pragma once

#include "engine/eng.h"

#include <memory>

class Game : public GL_engine::Application
{
public:
    void RegisterTypes() override;            //  类型登记
    bool AppInit() override;                  //  初始化
    void AppUpdate(float deltaTime) override; //  每帧更新
    void AppDestroy() override;               //  资源回收

private:
    std::shared_ptr<GL_engine::Scene> m_scene;
    GL_engine::GameObject *m_3DRoot = nullptr;

    GL_engine::GameObject *m_sun = nullptr;
};

#endif