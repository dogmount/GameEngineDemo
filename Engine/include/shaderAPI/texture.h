#ifndef MY_TEXTURE_H
#define MY_TEXTURE_H
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>

#include "glad/glad.h"

namespace GL_engine
{
    enum class TextureType
    {
        Texture2D,
        CubeMap
    };

    class Texture
    {
    public:
        //  2D纹理
        Texture(int width, int height, int numChannels, unsigned char *data);

        //  立方体纹理
        Texture(int width, int height, int numChannels, const std::vector<unsigned char *> &data);

        ~Texture();

        //  FBO空纹理构造
        Texture();
        static std::shared_ptr<Texture> CreateEmpty()
        {
            auto tex = std::make_shared<Texture>();
            return tex;
        }

        //  获取纹理ID
        GLuint GetID() const;

        //  获取纹理类型
        TextureType GetType() const;

        //  初始化2D纹理
        void Init(int width, int height, int numChannels, unsigned char *data);

        //  初始化立方体纹理
        void InitCubeTexture(int width, int height, int numChannels, const std::vector<unsigned char *> &data);

        //  获取纹理宽度/高度
        int GetWidth() const;
        int GetHeight() const;

        //  1、加载外置文件贴图
        static std::shared_ptr<Texture> Load(const std::string &path);

        //  2、加载内嵌贴图
        static std::shared_ptr<Texture> LoadGLBEmbeddedTexture(const void *data, size_t size);

        //  3、加载立方体贴图
        static std::shared_ptr<Texture> LoadCubeTexture(const std::string &path);

    private:
        int m_width = 0;        //  贴图宽
        int m_height = 0;       //  贴图高
        int m_numChannels = 0;  //  贴图颜色通道
        GLuint m_textureID = 0; //  纹理标识符

        TextureType m_textureType = TextureType::Texture2D; //  纹理类型

        //  立方体纹理文件名数组
        static constexpr std::array<const char *, 6> cubeFace{"right", "left", "top", "bottom", "front", "back"};
    };

    class TextureManager
    {
    public:
        std::shared_ptr<Texture> GetOrLoadTexture(const std::string &path); //  获取外置文件贴图

        std::shared_ptr<Texture> GetOrLoadEmbeddedTexture(const std::string &key, const void *data = nullptr, size_t size = 0); //  获取内嵌贴图

        std::shared_ptr<Texture> GetOrLoadCubeTexture(const std::string &path); //  获取外置立方体贴图

    private:
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures; //  纹理缓存
    };
}

#endif