#include "scene/components/audioListenerComponent.h"
#include "scene/gameObject.h"
#include "engine/Engine.h"

namespace GL_engine
{
    void AudioListenerComponent::Update(float deltaTime)
    {
        auto pos = m_owner->GetWorldPosition();
        Engine::GetInstance().GetAudioManager().SetListenerPosition(pos);
    }
}