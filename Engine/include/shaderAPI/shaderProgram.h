#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H
#pragma once

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace GL_engine
{
    class Texture;

    class ShaderProgram
    {
    public:
        ShaderProgram() = delete;
        ShaderProgram(const ShaderProgram &) = delete;            //  拷贝构造
        ShaderProgram &operator=(const ShaderProgram &) = delete; //  拷贝赋值

        //  禁止隐式类型转换
        explicit ShaderProgram(GLuint shaderProgramID);
        ~ShaderProgram();

        //  绑定着色器程序
        void ShaderBind();

        //  获取uniform变量位置
        GLint GetUniformLocation(const std::string &name);

        //  设置int变量
        void SetUniform(const std::string &name, int value);

        //  设置单float变量
        void SetUniform(const std::string &name, float value);

        //  设置双float变量
        void SetUniform(const std::string &name, float v0, float v1);
        void SetUniform(const std::string &name, const glm::vec2 &value);

        //  设置三float变量
        void SetUniform(const std::string &name, float v0, float v1, float v2);
        void SetUniform(const std::string &name, const glm::vec3 &value);

        //  设置四float变量
        void SetUniform(const std::string &name, float v0, float v1, float v2, float v3);
        void SetUniform(const std::string &name, const glm::vec4 &value);

        //  设置mat2变量
        void SetUniform(const std::string &name, const glm::mat2 &mat);

        //  设置mat3变量
        void SetUniform(const std::string &name, const glm::mat3 &mat);

        //  设置mat4变量
        void SetUniform(const std::string &name, const glm::mat4 &mat);

        //  设置纹理
        void SetTexture(const std::string &name, Texture *texture);

    private:
        //  着色器程序id
        GLuint m_shaderProgramID = 0;

        //  uniform位置缓存
        std::unordered_map<std::string, GLint> m_uniformLocationCache;

        //  纹理单元计数器
        int m_currentTextureUnit = 0;
    };
}

#endif