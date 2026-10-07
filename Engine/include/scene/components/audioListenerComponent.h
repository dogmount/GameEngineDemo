#ifndef AUDIO_LISTENER_COMPONENT_H
#define AUDIO_LISTENER_COMPONENT_H
#pragma once

#include "scene/component.h"

namespace GL_engine
{
    class AudioListenerComponent : public Component
    {
        COMPONENT(AudioListenerComponent)
    public:
        void Update(float deltaTime) override;
    };
}

#endif