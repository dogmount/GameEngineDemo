#ifndef MATERIAL_H
#define MATERIAL_H
#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include <glm/glm.hpp>

namespace GL_engine
{
    class ShaderProgram;
    class Texture;
    enum class PassTag;

    class Material
    {
    public:
        //  设置/获取着色器程序
        void SetShaderProgram(const std::shared_ptr<ShaderProgram> &ShaderProgram);
        ShaderProgram *GetShaderProgram();

        //  按passTag设置/获取着色器程序
        void SetVariantProgram(PassTag tag, const std::shared_ptr<ShaderProgram> &program);
        ShaderProgram *GetVariantProgram(PassTag tag);

        //  设置单浮点参数容器
        void SetFloat(const std::string &name, float value);

        //  设置双浮点参数容器
        void SetVec2(const std::string &name, const glm::vec2 &value);

        //  设置三浮点参数容器
        void SetVec3(const std::string &name, const glm::vec3 &value);

        //  设置纹理
        void SetTexture(const std::string &name, const std::shared_ptr<Texture> &texture);

        //  参数绑定
        void BindParams(ShaderProgram *program);

        //  程序绑定+上传参数
        void MaterialBind(PassTag pass);

        //  加载材质
        static std::shared_ptr<Material> Load(const std::string &path);

    private:
        std::shared_ptr<ShaderProgram> m_shaderProgram;

        std::unordered_map<PassTag, std::shared_ptr<ShaderProgram>> m_programByPass; //  PassTag -> ShaderProgram 变体

        //  浮点数容器
        std::unordered_map<std::string, float> m_floatParams;

        //  双浮点数容器
        std::unordered_map<std::string, glm::vec2> m_float2Params;

        //  三浮点数容器vec3
        std::unordered_map<std::string, glm::vec3> m_float3Params;

        //  纹理容器
        std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    };
}

#endif
