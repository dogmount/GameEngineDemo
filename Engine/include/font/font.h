#ifndef MY_FONT_H
#define MY_FONT_H
#pragma once

#include "shaderAPI/texture.h"

namespace GL_engine
{
    struct GlyphDescription
    {
        //  字符边框
        int x0, y0;
        int x1, y1;

        //  位图尺寸
        int width;
        int height;

        //  前进量（两个字符之间间距）
        int advance;
        int xOffset = 0;
        int yOffset = 0;
    };

    class Font
    {
    public:
        int GetSize() const;

        //  获取字形描述
        const GlyphDescription &GetGlyphDescription(char asciiCode) const;

        //  获取纹理
        const std::shared_ptr<Texture> &GetTexture() const;

    private:
        friend class FontManager;

        int m_size = 0;
        GlyphDescription m_descriptions[128]; //  字符集

        std::shared_ptr<Texture> m_texture;
    };
}

#endif