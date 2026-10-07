#ifndef SPRITE_COMPONENT_H
#define SPRITE_COMPONENT_H
#pragma once

#include <memory>

#include <glm/glm.hpp>
#include "scene/component.h"

namespace GL_engine
{
    class Texture;

    class SpriteComponent : public Component
    {
        COMPONENT(SpriteComponent)
    public:
        void LoadProperties(const nlohmann::json &json) override;

        void Update(float deltaTime) override;

        //  设置/获取纹理
        void SetTexture(const std::shared_ptr<Texture> &texture);
        const std::shared_ptr<Texture> &GetTexture() const;

        //  设置/获取颜色
        void SetColor(const glm::vec4 &color);
        const glm::vec4 &GetColor() const;

        //  设置/获取尺寸
        void SetSize(const glm::vec2 &size);
        const glm::vec2 &GetSize() const;

        //  设置/获取左下角纹理坐标
        void SetLowerLertUV(const glm::vec2 &uv);
        const glm::vec2 &GetLowerLertUV() const;

        //  设置/获取右上角纹理坐标
        void SetUpperRightUV(const glm::vec2 &uv);
        const glm::vec2 &GetUpperRightUV() const;

        //  设置纹理
        void SetUV(const glm::vec2 &lowerLeftUV, const glm::vec2 &upperRightUV);

        //  设置/获取中心点
        void SetPivot(const glm::vec2 &pivot);
        const glm::vec2 &GetPivot() const;

        //  设置/获取可见性状态
        void SetVisible(bool visible);
        bool IsVisible() const;

    private:
        std::shared_ptr<Texture> m_texture;         //  纹理
        glm::vec4 m_color = glm::vec4(1.0f);        //  颜色
        glm::vec2 m_size = glm::vec2(100.0f);       //  尺寸
        glm::vec2 m_lowerLeftUV = glm::vec2(0.0f);  //  左下角纹理坐标
        glm::vec2 m_upperRightUV = glm::vec2(1.0f); //  右上角纹理坐标
        glm::vec2 m_pivot = glm::vec2(0.5f);        //  中心点
        bool m_visible = true;                      //  可见性
    };
}

#endif