#ifndef GAME_SPAWNER_H
#define GAME_SPAWNER_H
#pragma once

#include <engine/eng.h>

#include <vector>

//  生成器开关("启动板"):
//  子弹命中后开始按间隔在场景中随机生成物体; 由另一块"停止板"命中后停止并清除全部生成物。
//  约定: 场景里命名为 "SpawnerStart"(本类型) 与 "SpawnerStop"(普通物体, 只需 Mesh + Physics)
class Spawner : public GL_engine::GameObject
{
    GAMEOBJECT(Spawner)
public:
    void Update(float deltaTime) override;

    //  子弹命中启动板时调用
    void StartSpawning();

    //  子弹命中停止板时调用
    void StopSpawningAndClear();

private:
    void SpawnOneRandom();

private:
    bool m_spawning = false;                        //  是否在持续生成
    float m_timer = 0.0f;                           //  生成计时
    std::vector<GL_engine::GameObject *> m_spawned; //  已生成物体(用于清除)
};

#endif