#ifndef TEXT_COMPONENT_H
#define TEXT_COMPONENT_H
#pragma once

#include "scene/components/ui/UIElementComponent.h"

#include <glm/glm.hpp>
#include <string>

namespace GL_engine
{
    class Font;

    class TextComponent : public UIElementComponent
    {
        COMPONENT_RELATE(TextComponent, UIElementComponent)
    public:
        void LoadProperties(const nlohmann::json &json) override;
        void Render(CanvasComponent *canvas) override;

        //  设置/获取文本
        void SetText(const std::string &text);
        const std::string &GetText() const;

        //  设置/获取字体颜色
        void SetColor(const glm::vec4 &color);
        const glm::vec4 &GetColor() const;

        //  设置/获取字体
        void SetFont(const std::shared_ptr<Font> &font);
        void SetFont(const std::string &path, int size);
        const std::shared_ptr<Font> &GetFont() const;

        //  获取轴点
        glm::vec2 GetPivotPos() const;

    private:
        std::string m_text;

        glm::vec4 m_color = glm::vec4(1.0f);

        std::shared_ptr<Font> m_font;
    };
}

#endif