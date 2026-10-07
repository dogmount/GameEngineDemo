#include "scene/components/spriteComponent.h"
#include "shaderAPI/texture.h"
#include "render/renderQueue.h"
#include "scene/gameObject.h"
#include "engine/Engine.h"

#include <string>

namespace GL_engine
{
    void SpriteComponent::LoadProperties(const nlohmann::json &json)
    {
        //  纹理
        const std::string texturePath = json.value("texture", "");
        if (auto texture = Texture::Load(texturePath))
        {
            SetTexture(texture);
        }

        //  颜色
        if (json.contains("color"))
        {
            auto &colorObj = json["color"];
            glm::vec4 color(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f),
                colorObj.value("a", 1.0f));

            SetColor(color);
        }

        //  尺寸
        if (json.contains("size"))
        {
            auto &sizeObj = json["size"];
            glm::vec2 size(
                sizeObj.value("x", 100.0f),
                sizeObj.value("y", 100.0f));

            SetSize(size);
        }

        //  左下角纹理坐标
        if (json.contains("lowerLeftUV"))
        {
            auto &uvObj = json["lowerLeftUV"];
            glm::vec2 uv(
                uvObj.value("u", 0.0f),
                uvObj.value("v", 0.0f));

            SetLowerLertUV(uv);
        }

        //  右上角纹理坐标
        if (json.contains("upperRightUV"))
        {
            auto &uvObj = json["upperRightUV"];
            glm::vec2 uv(
                uvObj.value("u", 1.0f),
                uvObj.value("v", 1.0f));

            SetUpperRightUV(uv);
        }

        //  中心点
        if (json.contains("pivot"))
        {
            auto &pivotObj = json["pivot"];
            glm::vec2 pivot(
                pivotObj.value("x", 0.5f),
                pivotObj.value("y", 0.5f));

            SetPivot(pivot);
        }
    }

    void SpriteComponent::Update(float deltaTime)
    {
        if (!m_texture || !m_visible)
        {
            return;
        }

        RenderCommand2D command;
        command.modelMatrix = GetOwner()->GetWorldTransform2D();
        command.texture = m_texture.get();
        command.color = m_color;
        command.size = m_size;
        command.lowerLeftUV = m_lowerLeftUV;
        command.upperRightUV = m_upperRightUV;
        command.pivot = m_pivot;

        auto &renderQueue = Engine::GetInstance().GetRenderQueue();
        renderQueue.SubmitRenderCommand(command);
    }

    //  设置/获取纹理
    void SpriteComponent::SetTexture(const std::shared_ptr<Texture> &texture)
    {
        m_texture = texture;
    }
    const std::shared_ptr<Texture> &SpriteComponent::GetTexture() const
    {
        return m_texture;
    }

    //  设置/获取颜色
    void SpriteComponent::SetColor(const glm::vec4 &color)
    {
        m_color = color;
    }
    const glm::vec4 &SpriteComponent::SpriteComponent::GetColor() const
    {
        return m_color;
    }

    //  设置/获取尺寸
    void SpriteComponent::SetSize(const glm::vec2 &size)
    {
        m_size = size;
    }
    const glm::vec2 &SpriteComponent::SpriteComponent::GetSize() const
    {
        return m_size;
    }

    //  设置/获取左下角纹理坐标
    void SpriteComponent::SetLowerLertUV(const glm::vec2 &uv)
    {
        m_lowerLeftUV = uv;
    }
    const glm::vec2 &SpriteComponent::GetLowerLertUV() const
    {
        return m_lowerLeftUV;
    }

    //  设置/获取右上角纹理坐标
    void SpriteComponent::SetUpperRightUV(const glm::vec2 &uv)
    {
        m_upperRightUV = uv;
    }
    const glm::vec2 &SpriteComponent::GetUpperRightUV() const
    {
        return m_upperRightUV;
    }

    //  设置纹理
    void SpriteComponent::SetUV(const glm::vec2 &lowerLeftUV, const glm::vec2 &upperRightUV)
    {
        m_lowerLeftUV = lowerLeftUV;
        m_upperRightUV = upperRightUV;
    }

    //  设置/获取中心点
    void SpriteComponent::SetPivot(const glm::vec2 &pivot)
    {
        m_pivot = pivot;
    }
    const glm::vec2 &SpriteComponent::GetPivot() const
    {
        return m_pivot;
    }

    //  设置/获取可见性状态
    void SpriteComponent::SetVisible(bool visible)
    {
        m_visible = visible;
    }
    bool SpriteComponent::IsVisible() const
    {
        return m_visible;
    }
}