#ifndef MY_FONT_MANAGER_H
#define MY_FONT_MANAGER_H
#pragma once

#include "font/font.h"

#include <memory>
#include <unordered_map>

typedef struct FT_LibraryRec_ *FT_Library;

namespace GL_engine
{
    class FontManager
    {
    public:
        ~FontManager();
        void Init();

        //  获取字体
        std::shared_ptr<Font> GetFont(const std::string &path, int size);

    private:
        FT_Library m_fontLibrary = nullptr;

        using FontFamily = std::unordered_map<int, std::shared_ptr<Font>>;//  字体族（尺寸映射，特定字体实例）

        std::unordered_map<std::string, FontFamily> m_fonts;
    };
}

#endif