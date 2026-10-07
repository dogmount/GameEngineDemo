#include "font/fontManager.h"
#include "engine/Engine.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace GL_engine
{
    FontManager::~FontManager()
    {
        if (m_fontLibrary)
        {
            FT_Done_FreeType(m_fontLibrary);
            m_fontLibrary = nullptr;
        }
    }

    void FontManager::Init()
    {
        FT_Error error = FT_Init_FreeType(&m_fontLibrary);
        if (error != FT_Err_Ok)
        {
            m_fontLibrary = nullptr;
            return;
        }
    }

    //  获取字体
    std::shared_ptr<Font> FontManager::GetFont(const std::string &path, int size)
    {
        auto fontFamilyIt = m_fonts.find(path);
        if (fontFamilyIt != m_fonts.end())
        {
            auto fontIt = fontFamilyIt->second.find(size);
            if (fontIt != fontFamilyIt->second.end())
            {
                return fontIt->second;
            }
        }

        auto buffer = Engine::GetInstance().GetFileSystem().LoadAssetFile(path);
        if (buffer.empty())
        {
            return nullptr;
        }

        FT_Face face;
        FT_Error result = FT_New_Memory_Face(
            m_fontLibrary, reinterpret_cast<FT_Byte *>(buffer.data()), buffer.size(), 0, &face);
        if (result != FT_Err_Ok)
        {
            return nullptr;
        }

        //  设置字体大小
        FT_Set_Pixel_Sizes(face, 0, size);

        const int lineHeight = face->size->metrics.height >> 6;
        int maxDimension = static_cast<int>(std::sqrt(128.0f) * (lineHeight + 1));
        int textureWidth = 1;
        while (textureWidth < maxDimension)
        {
            textureWidth <<= 1;
        }
        int textureHeight = textureWidth;

        const size_t stride = textureWidth * 4;
        const size_t totalBytes = static_cast<size_t>(textureWidth * textureHeight * 4);
        auto atlas = new unsigned char[totalBytes];
        std::memset(atlas, 0, totalBytes);

        int penX = 0;
        int penY = 0;

        auto font = std::make_shared<Font>();

        for (int c = 0; c < 128; ++c)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER) != FT_Err_Ok)
            {
                font->m_descriptions[c] = {0, 0, 0, 0, 0, 0, 0};
                continue;
            }

            FT_Bitmap &bmp = face->glyph->bitmap;

            if ((penX + static_cast<int>(bmp.width)) >= textureWidth)
            {
                penX = 0;
                penY += lineHeight + 1;
            }

            for (uint32_t row = 0; row < bmp.rows; ++row)
            {
                for (uint32_t col = 0; col < bmp.width; ++col)
                {
                    int x = penX + static_cast<int>(col);
                    int y = penY + static_cast<int>(row);
                    if (x < 0 || x >= textureWidth ||
                        y < 0 || y >= textureHeight)
                    {
                        continue;
                    }

                    const unsigned char value = bmp.buffer[row * bmp.pitch + col];
                    const size_t idx = static_cast<size_t>(y) * stride + x * 4;

                    atlas[idx + 0] = value;
                    atlas[idx + 1] = value;
                    atlas[idx + 2] = value;
                    atlas[idx + 3] = value;
                }
            }

            auto &gd = font->m_descriptions[c];
            gd.x0 = penX;
            gd.y0 = penY;
            gd.x1 = penX + static_cast<int>(bmp.width);
            gd.y1 = penY + static_cast<int>(bmp.rows);
            gd.width = static_cast<int>(bmp.width);
            gd.height = static_cast<int>(bmp.rows);
            gd.advance = (face->glyph->advance.x >> 6);
            gd.xOffset = static_cast<int>(face->glyph->bitmap_left);
            gd.yOffset = static_cast<int>(face->glyph->bitmap_top);

            penX += static_cast<int>(bmp.width + 1);
        }

        font->m_texture = std::make_shared<Texture>(textureWidth, textureHeight, 4, atlas);
        font->m_size = size;

        m_fonts[path][size] = font;

        delete[] atlas;
        FT_Done_Face(face);

        return font;
    }
}