#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H
#pragma once

#include "engine/common.h"

#include <memory>

struct ma_engine;

namespace GL_engine
{
    class AudioManager
    {
    public:
        AudioManager();
        ~AudioManager();

        bool Init();
        //  音频引擎获取器
        ma_engine *GetEngine();

        void SetListenerPosition(const glm::vec3 &pos);

    private:
        std::unique_ptr<ma_engine> m_engine;
    };
}

#endif