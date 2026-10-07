#include "shaderAPI/shaderAPI.h"
#include "shaderAPI/shaderProgram.h"
#include "render/material.h"
#include "render/mesh.h"
#include "engine/Engine.h"
#include "IO_file/file_system.h"

#include <iostream>

namespace GL_engine
{
    bool ShaderAPI::Init(FileSystem *fs)
    {
        glEnable(GL_DEPTH_TEST);
        m_fileSystem = fs;
        return true;
    }

    //  构建着色器程序
    std::shared_ptr<ShaderProgram> ShaderAPI::CreateShaderProgram(
        const std::string &vertexSource,
        const std::string &fragmentSource)
    {
        ShaderKey key{vertexSource, fragmentSource};
        auto it = m_shaderCache.find(key);
        if (it != m_shaderCache.end())
        {
            return it->second;
        }

        const char *vShaderCode = vertexSource.c_str();
        const char *fShaderCode = fragmentSource.c_str();

        // vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
        glCompileShader(vertexShader);
        if (!CheckCompileErrors(vertexShader, "VERTEX"))
        {
            glDeleteShader(vertexShader);
            return nullptr;
        }

        // fragment Shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
        glCompileShader(fragmentShader);
        if (!CheckCompileErrors(fragmentShader, "FRAGMENT"))
        {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return nullptr;
        }

        // 着色器程序
        GLuint programID = glCreateProgram();
        glAttachShader(programID, vertexShader);
        glAttachShader(programID, fragmentShader);

        glLinkProgram(programID);
        if (!CheckCompileErrors(programID, "PROGRAM"))
        {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return nullptr;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        auto shaderProgram = std::make_shared<ShaderProgram>(programID);
        m_shaderCache.emplace(key, shaderProgram);

        return shaderProgram;
    }

    //  获取默认着色器程序
    const std::shared_ptr<ShaderProgram> &ShaderAPI::GetDefaultProgram()
    {
        //  延迟初始化
        if (!m_defaultShaderProgram)
        {
            m_defaultShaderProgram = LoadShaderFromPath("Default_ShaderProgram", "shaders/default_3D/vertex.glsl", "shaders/default_3D/fragment.glsl");
        }

        return m_defaultShaderProgram;
    }

    //  获取默认2D着色器程序
    const std::shared_ptr<ShaderProgram> &ShaderAPI::GetDefaultProgram2D()
    {
        //  延迟初始化
        if (!m_defaultShaderProgram2D)
        {
            m_defaultShaderProgram2D = LoadShaderFromPath("Default2D_ShaderProgram", "shaders/default_2D/vertex.glsl", "shaders/default_2D/fragment.glsl");
        }

        return m_defaultShaderProgram2D;
    }

    //  获取默认UI着色器程序
    const std::shared_ptr<ShaderProgram> &ShaderAPI::GetDefaultProgramUI()
    {
        //  延迟初始化
        if (!m_defaultShaderProgramUI)
        {
            m_defaultShaderProgramUI = LoadShaderFromPath("DefaultUI_ShaderProgram", "shaders/default_ui/vertex.glsl", "shaders/default_ui/fragment.glsl");
        }

        return m_defaultShaderProgramUI;
    }

    //  创建VBO
    GLuint ShaderAPI::CreateVertexBuffer(const std::vector<float> &vertices)
    {
        GLuint VBO = 0;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        return VBO;
    }

    //  创建EBO
    GLuint ShaderAPI::CreateIndexBuffer(const std::vector<uint32_t> &indices)
    {
        GLuint EBO = 0;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        return EBO;
    }

    //  清空颜色/深度缓冲区
    void ShaderAPI::ClearBuffers()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    //  设置/获取视口
    void ShaderAPI::SetViewport(int x, int y, int width, int height)
    {
        glViewport(x, y, width, height);
        m_viewport.x = x;
        m_viewport.y = y;
        m_viewport.width = width;
        m_viewport.height = height;
    }
    const Rect &ShaderAPI::GetViewport() const
    {
        return m_viewport;
    }

    //  启用深度测试
    void ShaderAPI::SetDepthTestEnabled(bool enable)
    {
        if (enable)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
    }

    //  设置混合模式
    void ShaderAPI::SetBlendMode(BlendMode mode)
    {
        switch (mode)
        {
        case BlendMode::Disabled:
        {
            glDisable(GL_BLEND);
        }
        break;
        case BlendMode::Alpha:
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        break;
        case BlendMode::Additive:
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
        }
        break;
        case BlendMode::Multiply:
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
        }
        break;
        default:
            glDisable(GL_BLEND);
            break;
        }
    }

    //  设置屏幕背景颜色
    void ShaderAPI::SetClearColor(float red, float green, float blue, float alpha)
    {
        glClearColor(red, green, blue, alpha);
    }

    //  绑定着色器程序
    void ShaderAPI::BindShaderProgram(ShaderProgram *shaderProgram)
    {
        if (shaderProgram)
        {
            shaderProgram->ShaderBind();
        }
    }

    //  绑定材质
    void ShaderAPI::BindMaterial(Material *material, PassTag pass)
    {
        if (material)
        {
            material->MaterialBind(pass);
        }
    }

    //  绑定网格
    void ShaderAPI::BindMesh(Mesh *mesh)
    {
        if (mesh)
        {
            mesh->MeshBind();
        }
    }

    //  解绑网格
    void ShaderAPI::UnbindMesh(Mesh *mesh)
    {
        if (mesh)
        {
            mesh->MeshUnbind();
        }
    }

    //  绘制网格
    void ShaderAPI::DrawMesh(Mesh *mesh)
    {
        if (mesh)
        {
            mesh->MeshDraw();
        }
    }

    //  按名称获取着色器
    std::shared_ptr<ShaderProgram> ShaderAPI::GetShader(const std::string &name)
    {
        auto it = m_nameShaders.find(name);
        if (it != m_nameShaders.end())
        {
            return it->second;
        }

        return nullptr;
    }

    //  按名称与路径获取/加载着色器程序
    std::shared_ptr<ShaderProgram> ShaderAPI::GetOrLoadShader(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath)
    {
        auto existing = GetShader(name);
        if (existing)
        {
            return existing;
        }

        return LoadShaderFromPath(name, vertexPath, fragmentPath);
    }

    //  按名称注册关联着色器
    void ShaderAPI::RegisterShader(const std::string &name, std::shared_ptr<ShaderProgram> program)
    {
        if (!program)
        {
            return;
        }

        auto it = m_nameShaders.find(name);
        if (it != m_nameShaders.end() && it->second != program)
        {
            std::cerr << "Warning: Shader name '" << name << "' is being overwritten with a different program." << std::endl;
        }
        m_nameShaders[name] = program;
    }

    //  按名称与路径加载着色器程序
    std::shared_ptr<ShaderProgram> ShaderAPI::LoadShaderFromPath(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath)
    {
        if (!m_fileSystem)
        {
            std::cerr << "Error: FileSystem not injected into ShaderAPI!" << std::endl;
            return nullptr;
        }

        auto vertexSrc = m_fileSystem->LoadAssetFileText(vertexPath);
        auto fragmentSrc = m_fileSystem->LoadAssetFileText(fragmentPath);

        auto shaderProgram = CreateShaderProgram(vertexSrc, fragmentSrc);
        if (!shaderProgram)
        {
            return nullptr;
        }

        // 3. 注册名称映射（即使 program 是从 m_shaderCache 复用的也注册）
        RegisterShader(name, shaderProgram);
        return shaderProgram;
    }

    //  检测着色器程序编译是否发生错误
    bool ShaderAPI::CheckCompileErrors(GLuint unit, const std::string &type)
    {
        int success;
        char infoLog[512];
        if (type != "PROGRAM")
        {
            glGetShaderiv(unit, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(unit, 512, nullptr, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;

                return false;
            }
        }
        else
        {
            glGetProgramiv(unit, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(unit, 512, nullptr, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                return false;
            }
        }

        return true;
    }
}