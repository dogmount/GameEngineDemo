#include "shaderAPI/texture.h"
#include "engine/Engine.h"

#include <iostream>

// 纹理头文件
#define STB_IMAGE_IMPLEMENTATION
#include <tool/stb_image.h>

namespace GL_engine
{
    Texture::Texture(int width, int height, int numChannels, unsigned char *data)
        : m_width(width), m_height(height), m_numChannels(numChannels)
    {
        Init(width, height, numChannels, data);
        m_textureType = TextureType::Texture2D;
    }

    //  立方体纹理
    Texture::Texture(int width, int height, int numChannels, const std::vector<unsigned char *> &data)
        : m_width(width), m_height(height), m_numChannels(numChannels)
    {
        InitCubeTexture(width, height, numChannels, data);
        m_textureType = TextureType::CubeMap;
    }

    //  FBO空纹理构造
    Texture::Texture()
    {
        glGenTextures(1, &m_textureID);
        m_textureType = TextureType::Texture2D;
    }

    Texture::~Texture()
    {
        if (m_textureID > 0)
        {
            glDeleteTextures(1, &m_textureID);
        }
    }

    //  获取纹理ID
    GLuint Texture::GetID() const
    {
        return m_textureID;
    }

    //  获取纹理类型
    TextureType Texture::GetType() const
    {
        return m_textureType;
    }

    //  初始化2D
    void Texture::Init(int width, int height, int numChannels, unsigned char *data)
    {
        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        //  根据通道数改变格式参数
        GLint internalFormat = GL_RGB;
        GLenum format = GL_RGB;
        if (numChannels == 1)
        {
            internalFormat = GL_RED;
            format = GL_RED;
        }
        else if (numChannels == 2)
        {
            internalFormat = GL_RG;
            format = GL_RG;
        }
        else if (numChannels == 4)
        {
            internalFormat = GL_RGBA;
            format = GL_RGBA;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        // 生成多级渐远纹理
        glGenerateMipmap(GL_TEXTURE_2D);

        //  设置环绕
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        //  设置过滤方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    //  初始化立方体纹理
    void Texture::InitCubeTexture(int width, int height, int numChannels, const std::vector<unsigned char *> &data)
    {
        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

        //  根据通道数改变格式参数
        GLint internalFormat = GL_RGB;
        GLenum format = GL_RGB;
        if (numChannels == 1)
        {
            internalFormat = GL_RED;
            format = GL_RED;
        }
        else if (numChannels == 2)
        {
            internalFormat = GL_RG;
            format = GL_RG;
        }
        else if (numChannels == 4)
        {
            internalFormat = GL_RGBA;
            format = GL_RGBA;
        }

        for (unsigned i = 0; i < data.size(); ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data[i]);
        }

        //  设置环绕
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        //  设置过滤方式
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    //  获取纹理宽度/高度
    int Texture::GetWidth() const
    {
        return m_width;
    }
    int Texture::GetHeight() const
    {
        return m_height;
    }

    //  1、加载外置文件贴图
    std::shared_ptr<Texture> Texture::Load(const std::string &path)
    {
        int width, height, numChannels;

        //  文件系统读取纹理
        auto &fs = Engine::GetInstance().GetFileSystem();
        auto fullPath = fs.GetAssetsFolder() / path;

        //  检测路径是否存在
        if (!std::filesystem::exists(fullPath))
        {
            return nullptr;
        }

        std::shared_ptr<Texture> result;

        stbi_set_flip_vertically_on_load(true); //  图像Y轴翻转

        unsigned char *data = stbi_load(fullPath.string().c_str(), &width, &height, &numChannels, 0);

        if (data)
        {
            result = std::make_shared<Texture>(width, height, numChannels, data);
        }
        else
        {
            std::cerr << "Failed to load texture" << std::endl;
            result = nullptr;
        }

        stbi_image_free(data); // 释放数据
        return result;
    }

    //  获取外置文件贴图
    std::shared_ptr<Texture> TextureManager::GetOrLoadTexture(const std::string &path)
    {
        auto it = m_textures.find(path);
        if (it != m_textures.end())
        {
            return it->second;
        }

        auto texture = Texture::Load(path);
        m_textures[path] = texture;
        return texture;
    }

    //  2、加载内嵌贴图
    std::shared_ptr<Texture> Texture::LoadGLBEmbeddedTexture(const void *data, size_t size)
    {
        if (!data || size == 0)
        {
            return nullptr;
        }

        int width, height, numChannels;

        std::shared_ptr<Texture> result;

        stbi_set_flip_vertically_on_load(true); //  图像Y轴翻转

        unsigned char *pixels = stbi_load_from_memory(static_cast<const unsigned char *>(data), static_cast<int>(size), &width, &height, &numChannels, 0);

        if (pixels)
        {
            result = std::make_shared<Texture>(width, height, numChannels, pixels);
        }
        else
        {
            std::cerr << "Failed to load Embedded texture" << std::endl;
            result = nullptr;
        }

        stbi_image_free(pixels); // 释放数据
        return result;
    }

    //  获取内嵌贴图
    std::shared_ptr<Texture> TextureManager::GetOrLoadEmbeddedTexture(const std::string &key, const void *data, size_t size)
    {
        auto it = m_textures.find(key);
        if (it != m_textures.end())
        {
            return it->second;
        }

        auto texture = Texture::LoadGLBEmbeddedTexture(data, size);
        m_textures[key] = texture;
        return texture;
    }

    //  3、加载立方体贴图
    std::shared_ptr<Texture> Texture::LoadCubeTexture(const std::string &path)
    {
        stbi_set_flip_vertically_on_load(false); //  关闭图像Y轴翻转

        std::vector<unsigned char *> cubeData(6);

        int width, height, numChannels;

        auto &fs = Engine::GetInstance().GetFileSystem();
        auto folderPath = fs.GetAssetsFolder() / path;

        bool success = true;

        //  第一步：加载立方体贴图
        for (size_t i = 0; i < 6; ++i)
        {
            std::string base = std::string(cubeFace[i]);
            std::filesystem::path pngPath = std::filesystem::path(folderPath) / (base + ".png");
            std::filesystem::path jpgPath = std::filesystem::path(folderPath) / (base + ".jpg");

            std::string fullPath;
            if (std::filesystem::exists(pngPath))
            {
                fullPath = pngPath.string();
            }
            else if (std::filesystem::exists(jpgPath))
            {
                fullPath = jpgPath.string();
            }
            else
            {
                std::cerr << "Cube face not found: " << base << std::endl;
                success = false;
                break;
            }

            int w, h, c;
            unsigned char *imageData = stbi_load(fullPath.c_str(), &w, &h, &c, 0);
            if (!imageData)
            {
                std::cerr << "Failed to load cube face: " << fullPath << std::endl;
                success = false;
                break;
            }

            if (i == 0) //  记录第一张贴图尺寸信息
            {
                width = w;
                height = h;
                numChannels = c;
            }
            else //  检查尺寸信息一致性
            {
                if (w != width || h != height || c != numChannels)
                {
                    std::cerr << "Cube face size/channel mismatch!" << std::endl;
                    stbi_image_free(imageData);
                    success = false;
                    break;
                }
            }

            cubeData[i] = imageData;
        }

        if (!success)
        {
            for (auto data : cubeData)
            {
                if (data)
                {
                    stbi_image_free(data);
                }
            }
            return nullptr;
        }

        std::shared_ptr<Texture> result = std::make_shared<Texture>(width, height, numChannels, cubeData);

        for (auto data : cubeData)
        {
            if (data)
            {
                stbi_image_free(data);
            }
        }
        return result;
    }

    //  获取外置立方体贴图
    std::shared_ptr<Texture> TextureManager::GetOrLoadCubeTexture(const std::string &path)
    {
        auto it = m_textures.find(path);
        if (it != m_textures.end())
        {
            return it->second;
        }

        auto texture = Texture::LoadCubeTexture(path);
        m_textures[path] = texture;
        return texture;
    }
}