#ifndef SHADER_API_H
#define SHADER_API_H
#pragma once

#include "glad/glad.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace GL_engine
{
    class FileSystem;
    class ShaderProgram;
    class Material;
    class Mesh;
    enum class PassTag;

    //  混合模式
    enum class BlendMode
    {
        Disabled,
        Alpha,
        Additive,
        Multiply
    };

    //  矩形数据
    struct Rect
    {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
    };

    //  着色器源
    struct ShaderKey
    {
        std::string vertexSource;
        std::string fragmentSource;

        bool operator==(const ShaderKey &other) const
        {
            return vertexSource == other.vertexSource && fragmentSource == other.fragmentSource;
        }
    };

    //  着色器哈希
    struct ShaderKeyHash
    {
        std::size_t operator()(const ShaderKey &key) const
        {
            std::size_t h1 = std::hash<std::string>{}(key.vertexSource);
            std::size_t h2 = std::hash<std::string>{}(key.fragmentSource);
            return h1 ^ (h2 << 1);
        }
    };

    class ShaderAPI
    {
    public:
        bool Init(FileSystem *fs);

        std::shared_ptr<ShaderProgram> CreateShaderProgram(
            const std::string &vertex,
            const std::string &fragment);

        //  获取默认3D着色器程序
        const std::shared_ptr<ShaderProgram> &GetDefaultProgram();

        //  获取默认2D着色器程序
        const std::shared_ptr<ShaderProgram> &GetDefaultProgram2D();

        //  获取默认UI着色器程序
        const std::shared_ptr<ShaderProgram> &GetDefaultProgramUI();

        //  创建VBO
        GLuint CreateVertexBuffer(const std::vector<float> &vertices);

        //  创建EBO
        GLuint CreateIndexBuffer(const std::vector<uint32_t> &indices);

        //  清空颜色/深度缓冲区
        void ClearBuffers();

        //  设置/获取视口
        void SetViewport(int x, int y, int width, int height);
        const Rect &GetViewport() const;

        //  启用深度测试
        void SetDepthTestEnabled(bool enable);

        //  设置混合模式
        void SetBlendMode(BlendMode mode);

        //  设置屏幕背景颜色
        void SetClearColor(float red, float green, float blue, float alpha);

        //  绑定着色器程序
        void BindShaderProgram(ShaderProgram *shaderProgram);

        //  绑定材质
        void BindMaterial(Material *material, PassTag pass);

        //  绑定网格
        void BindMesh(Mesh *mesh);

        //  解绑网格
        void UnbindMesh(Mesh *mesh);

        //  绘制网格
        void DrawMesh(Mesh *mesh);

        //  按名称获取着色器
        std::shared_ptr<ShaderProgram> GetShader(const std::string &name);

        //  按名称与路径获取/加载着色器程序
        std::shared_ptr<ShaderProgram> GetOrLoadShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath);

        //  按名称注册关联着色器
        void RegisterShader(const std::string &name, std::shared_ptr<ShaderProgram> program);

        //  按名称与路径加载着色器程序
        std::shared_ptr<ShaderProgram> LoadShaderFromPath(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath);

    private:
        //  检测着色器程序编译错误
        bool CheckCompileErrors(GLuint shader, const std::string &type);

        //  矩形视口数据
        Rect m_viewport;

        //  默认3D着色器
        std::shared_ptr<ShaderProgram> m_defaultShaderProgram;

        //  默认2D着色器
        std::shared_ptr<ShaderProgram> m_defaultShaderProgram2D;

        //  默认UI着色器
        std::shared_ptr<ShaderProgram> m_defaultShaderProgramUI;

        //  着色器程序缓存
        std::unordered_map<ShaderKey, std::shared_ptr<ShaderProgram>, ShaderKeyHash> m_shaderCache;

        //  名称映射着色器程序
        std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_nameShaders;

        FileSystem *m_fileSystem = nullptr;
    };
}

#endif