#include "font/font.h"

namespace GL_engine
{
    int Font::GetSize() const
    {
        return m_size;
    }

    //  获取字形描述
    const GlyphDescription &Font::GetGlyphDescription(char asciiCode) const
    {
        return m_descriptions[static_cast<unsigned char>(asciiCode)];
    }

    //  获取纹理
    const std::shared_ptr<Texture> &Font::GetTexture() const
    {
        return m_texture;
    }
}