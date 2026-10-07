#include "shaderAPI/shaderProgram.h"
#include <glm/gtc/type_ptr.hpp>
#include "shaderAPI/texture.h"

namespace GL_engine
{
    ShaderProgram::ShaderProgram(GLuint shaderProgramID) : m_shaderProgramID(shaderProgramID)
    {
    }

    ShaderProgram::~ShaderProgram()
    {
        glDeleteProgram(m_shaderProgramID);
    }

    void ShaderProgram::ShaderBind()
    {
        glUseProgram(m_shaderProgramID);
        m_currentTextureUnit = 0;
    }

    //  获取着色器程序中uniform变量位置
    GLint ShaderProgram::GetUniformLocation(const std::string &name)
    {
        auto it = m_uniformLocationCache.find(name);
        if (it != m_uniformLocationCache.end())
        {
            return it->second;
        }

        GLint location = glGetUniformLocation(m_shaderProgramID, name.c_str());
        m_uniformLocationCache[name] = location;

        return location;
    }

    //  设置int变量
    void ShaderProgram::SetUniform(const std::string &name, int value)
    {
        auto location = GetUniformLocation(name);
        glUniform1i(location, value);
    }

    //  设置单Float变量
    void ShaderProgram::SetUniform(const std::string &name, float value)
    {
        auto location = GetUniformLocation(name);
        glUniform1f(location, value);
    }

    //  设置双Float变量
    void ShaderProgram::SetUniform(const std::string &name, float v0, float v1)
    {
        auto location = GetUniformLocation(name);
        glUniform2f(location, v0, v1);
    }

    void ShaderProgram::SetUniform(const std::string &name, const glm::vec2 &value)
    {
        auto location = GetUniformLocation(name);
        glUniform2fv(location, 1, glm::value_ptr(value));
    }

    //  设置vec3变量
    void ShaderProgram::SetUniform(const std::string &name, float v0, float v1, float v2)
    {
        auto location = GetUniformLocation(name);
        glUniform3f(location, v0, v1, v2);
    }
    void ShaderProgram::SetUniform(const std::string &name, const glm::vec3 &value)
    {
        auto location = GetUniformLocation(name);
        glUniform3fv(location, 1, glm::value_ptr(value));
    }

    //  设置vec4变量
    void ShaderProgram::SetUniform(const std::string &name, float v0, float v1, float v2, float v3)
    {
        auto location = GetUniformLocation(name);
        glUniform4f(location, v0, v1, v2, v3);
    }
    void ShaderProgram::SetUniform(const std::string &name, const glm::vec4 &value)
    {
        auto location = GetUniformLocation(name);
        glUniform4fv(location, 1, glm::value_ptr(value));
    }

    //  设置mat2变量
    void ShaderProgram::SetUniform(const std::string &name, const glm::mat2 &mat)
    {
        auto location = GetUniformLocation(name);
        glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }

    //  设置mat3变量
    void ShaderProgram::SetUniform(const std::string &name, const glm::mat3 &mat)
    {
        auto location = GetUniformLocation(name);
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }

    //  设置mat4变量
    void ShaderProgram::SetUniform(const std::string &name, const glm::mat4 &mat)
    {
        auto location = GetUniformLocation(name);
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }

    //  设置纹理
    void ShaderProgram::SetTexture(const std::string &name, Texture *texture)
    {
        if (!texture)
        {
            return;
        }
        auto location = GetUniformLocation(name);

        glActiveTexture(GL_TEXTURE0 + m_currentTextureUnit);

        switch (texture->GetType()) // 2) 按类型选择绑定目标
        {
        case TextureType::CubeMap:
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture->GetID());
            break;
        case TextureType::Texture2D:
        default:
            glBindTexture(GL_TEXTURE_2D, texture->GetID());
            break;
        }

        glUniform1i(location, m_currentTextureUnit);

        //  绑定纹理后递增
        ++m_currentTextureUnit;
    }
}