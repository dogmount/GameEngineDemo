#include "scene/components/animationComponent.h"
#include "scene/gameObject.h"

#include <algorithm>

namespace GL_engine
{
    void AnimationComponent::Update(float deltaTime)
    {
        //  无动画/未播放直接返回
        if (!m_clip)
        {
            return;
        }
        if (!m_isPlaying)
        {
            return;
        }

        m_time += deltaTime; //  累积时间
        if (m_time > m_clip->duration)
        {
            //  循环播放
            if (m_looping)
            {
                m_time = std::fmod(m_time, m_clip->duration); // 浮点数取模运算
            }
            else
            {
                m_time = 0.0f;
                m_isPlaying = false;
                return;
            }
        }

        for (auto &binding : m_bindings)
        {
            auto &obj = binding.first;
            auto &trackIndices = binding.second->trackIndices;
            for (auto i : trackIndices)
            {
                auto &track = m_clip->tracks[i];
                if (!track.positions.empty())
                {
                    auto pos = Interpolate(track.positions, m_time);
                    obj->SetPosition(pos);
                }
                if (!track.rotations.empty())
                {
                    auto rot = Interpolate(track.rotations, m_time);
                    obj->SetRotation(rot);
                }
                if (!track.scales.empty())
                {
                    auto scale = Interpolate(track.scales, m_time);
                    obj->SetScale(scale);
                }
            }
        }
    }

    //  指定动画剪辑
    void AnimationComponent::SetClip(AnimationClip *clip)
    {
        m_clip = clip;
        BuildBindings();
    }

    //  缓存动画剪辑
    void AnimationComponent::RegisterClip(const std::string &name, const std::shared_ptr<AnimationClip> &clip)
    {
        m_clips[name] = clip;
    }

    //  播放控制
    void AnimationComponent::Play(const std::string &name, bool loop)
    {
        if (m_clip && m_clip->name == name)
        {
            m_time = 0.0f;
            m_isPlaying = true;
            m_looping = loop;
        }
        else
        {
            auto it = m_clips.find(name);
            if (it != m_clips.end())
            {
                SetClip(it->second.get());
                m_time = 0.0f;
                m_isPlaying = true;
                m_looping = loop;
            }
        }
    }

    //  获取播放状态
    bool AnimationComponent::IsPlaying() const
    {
        return m_isPlaying;
    }

    //  绑定动画
    void AnimationComponent::BuildBindings()
    {
        m_bindings.clear();
        if (!m_clip)
        {
            return;
        }

        for (size_t i = 0; i < m_clip->tracks.size(); ++i)
        {
            auto &track = m_clip->tracks[i];
            auto targetObj = m_owner->FindChildByName(track.targetName);
            if (targetObj)
            {
                auto it = m_bindings.find(targetObj);
                if (it != m_bindings.end())
                {
                    it->second->trackIndices.push_back(i);
                }
                else
                {
                    auto binding = std::make_unique<ObjectBinding>();
                    binding->object = targetObj;
                    binding->trackIndices.push_back(i);
                    m_bindings.emplace(targetObj, std::move(binding));
                }
            }
        }
    }

    //  vec3插值
    glm::vec3 AnimationComponent::Interpolate(const std::vector<KeyFrameVec3> &keys, float time)
    {

        //  空列表保护
        if (keys.empty())
        {
            return glm::vec3(0.0f);
        }

        //  单帧处理
        if (keys.size() == 1)
        {
            return keys[0].value;
        }

        //  时间边界裁剪
        if (time <= keys.front().time)
        {
            return keys.front().value;
        }
        if (time >= keys.back().time)
        {
            return keys.back().value;
        }

        size_t i0 = 0;
        size_t i1 = 0;

        //  寻找插值区间（顺序搜索）
        // for (size_t i = 1; i < keys.size(); ++i)
        // {
        //     if (time <= keys[i].time)
        //     {
        //         i1 = i;
        //         break;
        //     }
        // }
        // i0 = i1 > 0 ? i1 - 1 : 0;

        //  寻找插值区间（二分查找）
        size_t lower = 0;
        size_t higher = keys.size(); // 注意：higher 是开区间上限（size），不参与比较
        while (lower < higher)
        {
            size_t mid = (lower + higher) / 2;
            if (keys[mid].time < time)
            {
                lower = mid + 1;
            }
            else
            {
                higher = mid;
            }
        }
        i1 = lower;
        i0 = i1 > 0 ? i1 - 1 : 0;

        //  计算插值因子并执行混合
        if (time >= keys[i0].time && time <= keys[i1].time)
        {
            float deltaTime = keys[i1].time - keys[i0].time;
            float k = (time - keys[i0].time) / deltaTime;

            return glm::mix(keys[i0].value, keys[i1].value, k);
        }
        return keys.back().value;
    }

    //  quat插值
    glm::quat AnimationComponent::Interpolate(const std::vector<KeyFrameQuat> &keys, float time)
    {
        //  空列表保护
        if (keys.empty())
        {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }

        //  单帧处理
        if (keys.size() == 1)
        {
            return keys[0].value;
        }

        //  时间边界裁剪
        if (time <= keys.front().time)
        {
            return keys.front().value;
        }
        if (time >= keys.back().time)
        {
            return keys.back().value;
        }

        size_t i0 = 0;
        size_t i1 = 0;

        //  寻找插值区间（顺序搜索）
        // for (size_t i = 1; i < keys.size(); ++i)
        // {
        //     if (time <= keys[i].time)
        //     {
        //         i1 = i;
        //         break;
        //     }
        // }
        // i0 = i1 > 0 ? i1 - 1 : 0;

        //  寻找插值区间（二分查找）
        size_t lower = 0;
        size_t higher = keys.size(); // 注意：higher 是开区间上限（size），不参与比较
        while (lower < higher)
        {
            size_t mid = (lower + higher) / 2;
            if (keys[mid].time < time)
            {
                lower = mid + 1;
            }
            else
            {
                higher = mid;
            }
        }
        i1 = lower;
        i0 = i1 > 0 ? i1 - 1 : 0;

        //  计算插值因子并执行混合
        if (time >= keys[i0].time && time <= keys[i1].time)
        {
            float deltaTime = keys[i1].time - keys[i0].time;
            float k = (time - keys[i0].time) / deltaTime;

            return glm::slerp(keys[i0].value, keys[i1].value, k);
        }
        return keys.back().value;
    }
}