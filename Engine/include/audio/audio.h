#ifndef MY_AUDIO_H
#define MY_AUDIO_H
#pragma once

#include "engine/common.h"

#include <memory>
#include <string>
#include <vector>

struct ma_sound;   //  播放器
struct ma_decoder; //  解码器

namespace GL_engine
{
    class Audio
    {
    public:
        ~Audio();

        //  设定声音位置
        void SetPositon(const glm::vec3 &postition);

        void Play(bool loop = false);
        void Stop();
        bool IsPlaying() const;

        //  设置/获取音量
        void SetVolume(float volume);
        float GetVolume() const;

        //  音频加载
        static std::shared_ptr<Audio> Load(const std::string &path);

    private:
        std::unique_ptr<ma_sound> m_sound;
        std::unique_ptr<ma_decoder> m_decoder;
        std::vector<char> m_buffer;
    };
}

#endif