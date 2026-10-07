#ifndef AUDIO_COMPONENT_H
#define AUDIO_COMPONENT_H
#pragma once

#include "scene/component.h"
#include "audio/audio.h"

namespace GL_engine
{
    class AudioComponent : public Component
    {
        COMPONENT(AudioComponent)
    public:
        //  加载属性
        void LoadProperties(const nlohmann::json &json) override;

        void Update(float deltaTime) override;

        //  登记Audio
        void RegisterAudio(const std::string &name, std::shared_ptr<Audio> &clip);

        void Play(const std::string &name, bool loop = false);
        void Stop(const std::string &name);
        bool IsPlaying(const std::string &name);

    private:
        std::unordered_map<std::string, std::shared_ptr<Audio>> m_clips;
    };
}

#endif