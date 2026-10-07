#include "audio/audioManager.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace GL_engine
{
    AudioManager::AudioManager()
    {
        m_engine = std::make_unique<ma_engine>();
    }
    AudioManager::~AudioManager()
    {
        if (m_engine)
        {
            //  销毁音频引擎
            ma_engine_uninit(m_engine.get());
        }
    }

    bool AudioManager::Init()
    {
        auto result = ma_engine_init(nullptr, m_engine.get());
        return result == MA_SUCCESS;
    }
    //  音频引擎获取器
    ma_engine *AudioManager::GetEngine()
    {
        return m_engine.get();
    }

    void AudioManager::SetListenerPosition(const glm::vec3 &pos)
    {
        //  设置监听器位置
        if (m_engine)
        {
            ma_engine_listener_set_position(m_engine.get(), 0, pos.x, pos.y, pos.z);
        }
    }
}