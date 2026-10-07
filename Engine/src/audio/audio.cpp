#include "audio/audio.h"
#include "engine/Engine.h"

#include <miniaudio.h>

namespace GL_engine
{
    Audio::~Audio()
    {
        if (m_sound)
        {
            ma_sound_uninit(m_sound.get());
        }

        if (m_decoder)
        {
            ma_decoder_uninit(m_decoder.get());
        }
    }

    //  设定声音位置
    void Audio::SetPositon(const glm::vec3 &postition)
    {
        if (m_sound)
        {
            ma_sound_set_position(m_sound.get(), postition.x, postition.y, postition.z);
        }
    }
    void Audio::Play(bool loop)
    {
        if (m_sound)
        {
            ma_sound_start(m_sound.get());
            auto looping = loop ? MA_TRUE : MA_FALSE;
            ma_sound_set_looping(m_sound.get(), looping);
        }
    }
    void Audio::Stop()
    {
        if (m_sound)
        {
            ma_sound_stop(m_sound.get());
            ma_sound_seek_to_pcm_frame(m_sound.get(), 0); //  将播放位置设置到开头
        }
    }
    bool Audio::IsPlaying() const
    {
        if (m_sound)
        {
            return ma_sound_is_playing(m_sound.get());
        }
        return false;
    }

    //  设置/获取音量
    void Audio::SetVolume(float volume)
    {
        if (m_sound)
        {
            ma_sound_set_volume(m_sound.get(), std::clamp(volume, 0.0f, 1.0f));
        }
    }
    float Audio::GetVolume() const
    {
        if (m_sound)
        {
            return ma_sound_get_volume(m_sound.get());
        }
        return 0.0f;
    }

    //  音频加载
    std::shared_ptr<Audio> Audio::Load(const std::string &path)
    {
        auto buffer = Engine::GetInstance().GetFileSystem().LoadAssetFile(path);
        auto engine = Engine::GetInstance().GetAudioManager().GetEngine();

        //  初始化audio组件
        auto audio = std::make_shared<Audio>();
        audio->m_sound = std::make_unique<ma_sound>();
        audio->m_buffer = buffer;
        audio->m_decoder = std::make_unique<ma_decoder>();

        //  初始化解码器，参数列表（音频文件指针，音频大小，高级配置（如解码格式），解码器指针
        auto result = ma_decoder_init_memory(audio->m_buffer.data(), audio->m_buffer.size(), nullptr, audio->m_decoder.get());
        if (result != MA_SUCCESS)
        {
            return nullptr;
        }

        //  绑定音频，参数列表（音频引擎指针，解码器指针，标志位（默认0），可选项，声音对象）
        result = ma_sound_init_from_data_source(
            engine,
            audio->m_decoder.get(),
            0,
            NULL,
            audio->m_sound.get());
        if (result != MA_SUCCESS)
        {
            return nullptr;
        }

        //  启用 3D 空间化
        ma_sound_set_spatialization_enabled(audio->m_sound.get(), MA_TRUE);
        return audio;
    }
}