#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H
#pragma once

#include "scene/component.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace GL_engine
{
    //  vec3关键帧
    struct KeyFrameVec3
    {
        float time = 0.0f;
        glm::vec3 value = glm::vec3(0.0f);
    };

    //  quat关键帧
    struct KeyFrameQuat
    {
        float time = 0.0f;
        glm::quat value = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    };

    //  变换轨道
    struct TransformTrack
    {
        std::string targetName;
        std::vector<KeyFrameVec3> positions;
        std::vector<KeyFrameQuat> rotations;
        std::vector<KeyFrameVec3> scales;
    };

    //  动画片段
    struct AnimationClip
    {
        std::string name;
        float duration = 0.0f;
        bool looping = true;
        std::vector<TransformTrack> tracks;
    };

    //  动画对象绑定
    struct ObjectBinding
    {
        GameObject *object = nullptr;
        std::vector<size_t> trackIndices;
    };

    class AnimationComponent : public Component
    {
        COMPONENT(AnimationComponent)

    public:
        void Update(float deltaTime) override;

        //  指定动画片段
        void SetClip(AnimationClip *clip);

        //  缓存动画片段
        void RegisterClip(const std::string &name, const std::shared_ptr<AnimationClip> &clip);

        //  播放控制
        void Play(const std::string &name, bool loop = true);

        //  获取播放状态
        bool IsPlaying() const;

    private:
        //  绑定动画
        void BuildBindings();

        //  vec3插值
        glm::vec3 Interpolate(const std::vector<KeyFrameVec3> &keys, float time);
        //  quat插值
        glm::quat Interpolate(const std::vector<KeyFrameQuat> &keys, float time);

    private:
        AnimationClip *m_clip = nullptr; //  动画片段
        float m_time = 0.0f;             //  动画时间
        bool m_looping = true;           //  是否循环
        bool m_isPlaying = false;        //  是否正在播放

        std::unordered_map<std::string, std::shared_ptr<AnimationClip>> m_clips;     //  缓存已加载的动画片段
        std::unordered_map<GameObject *, std::unique_ptr<ObjectBinding>> m_bindings; //  记录对象与动画映射关系
    };
}

#endif