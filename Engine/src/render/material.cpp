#include "render/material.h"
#include "shaderAPI/shaderProgram.h"
#include "shaderAPI/texture.h"
#include "engine/Engine.h"
#include "nlohmann/json.hpp"
#include "render/renderCommon.h"

#include <iostream>

namespace GL_engine
{
    //  意图: .mat 中 "pass" 字段 → PassTag
    PassTag PassTagFromString(const std::string &s)
    {
        if (s == "depth")
        {
            return PassTag::Depth;
        }
        if (s == "shadow")
        {
            return PassTag::Shadow;
        }
        if (s == "gbuffer")
        {
            return PassTag::GBuffer;
        }
        if (s == "custom")
        {
            return PassTag::Custom;
        }
        return PassTag::Forward;
    }

    //  意图: 读取并编译一段 shader(vertex+fragment 路径) → program
    std::shared_ptr<ShaderProgram> LoadProgram(const std::string &name, const std::string &vertexPath, const std::string &fragmentPath)
    {
        if (vertexPath.empty() || fragmentPath.empty())
        {
            return nullptr;
        }
        auto &fs = Engine::GetInstance().GetFileSystem();
        auto vertexSrc = fs.LoadAssetFileText(vertexPath);
        auto fragmentSrc = fs.LoadAssetFileText(fragmentPath);

        auto &shaderApi = Engine::GetInstance().GetShaderAPI();

        if (vertexSrc.empty() || fragmentSrc.empty())
        {
            return nullptr;
        }

        auto shaderProgram = shaderApi.CreateShaderProgram(vertexSrc, fragmentSrc); //  ShaderAPI 内部按源码去重(m_shaderCache), 相同源码不会重复编译
        if (!shaderProgram)
        {
            return nullptr;
        }
        shaderApi.RegisterShader(name, shaderProgram);

        return shaderProgram;
    }

    void Material::SetShaderProgram(const std::shared_ptr<ShaderProgram> &shaderProgram)
    {
        m_shaderProgram = shaderProgram;
    }

    //  获取着色器程序
    ShaderProgram *Material::GetShaderProgram()
    {
        return m_shaderProgram.get();
    }

    //  按passTag设置着色器程序
    void Material::SetVariantProgram(PassTag tag, const std::shared_ptr<ShaderProgram> &program)
    {
        if (!program)
        {
            return;
        }
        m_programByPass[tag] = program;
    }

    //  按passTag获取着色器程序
    ShaderProgram *Material::GetVariantProgram(PassTag tag)
    {
        auto it = m_programByPass.find(tag);
        if (it != m_programByPass.end())
        {
            return it->second.get();
        }

        return m_shaderProgram.get();
    }

    //  设置单浮点参数
    void Material::SetFloat(const std::string &name, float value)
    {
        m_floatParams[name] = value;
    }

    //  设置双浮点参数
    void Material::SetVec2(const std::string &name, const glm::vec2 &value)
    {
        m_float2Params[name] = value;
    }

    void Material::SetVec3(const std::string &name, const glm::vec3 &value)
    {
        m_float3Params[name] = value;
    }

    //  设置纹理
    void Material::SetTexture(const std::string &name, const std::shared_ptr<Texture> &texture)
    {
        m_textures[name] = texture;
    }

    //  参数绑定
    void Material::BindParams(ShaderProgram *program)
    {
        if (!program)
        {
            return;
        }

        for (auto &param : m_floatParams)
        {
            program->SetUniform(param.first, param.second);
        }

        for (auto &param : m_float2Params)
        {
            program->SetUniform(param.first, param.second);
        }

        for (auto &param : m_float3Params)
        {
            program->SetUniform(param.first, param.second);
        }

        for (auto &param : m_textures)
        {
            program->SetTexture(param.first, param.second.get());
        }
    }

    //  程序绑定+上传参数
    void Material::MaterialBind(PassTag pass)
    {
        ShaderProgram *program = GetVariantProgram(pass);
        if (!program)
        {
            return;
        }
        program->ShaderBind();
        BindParams(program);
    }

//  旧版正向渲染
//  绑定着色器程序(将uniform参数传入着色器程序)
#if 0
    void Material::MaterialBind()
    {
        if (!m_shaderProgram)
        {
            return;
        }

        m_shaderProgram->ShaderBind(); //  绑定着色器(纹理单元置为0)

        for (auto &param : m_floatParams)
        {
            m_shaderProgram->SetUniform(param.first, param.second);
        }

        for (auto &param : m_float2Params)
        {
            m_shaderProgram->SetUniform(param.first, param.second);
        }

        for (auto &param : m_float3Params)
        {
            m_shaderProgram->SetUniform(param.first, param.second);
        }

        for (auto &param : m_textures)
        {
            m_shaderProgram->SetTexture(param.first, param.second.get());
        }
    }
#endif

    //  加载材质
    std::shared_ptr<Material> Material::Load(const std::string &path)
    {
        auto contents = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);

        if (contents.empty())
        {
            return nullptr;
        }

        //  解析json
        nlohmann::json json = nlohmann::json::parse(contents);
        std::shared_ptr<Material> result;

        //  1、默认shader(Forward/兜底)
        // if (json.contains("shader") && json["shader"].is_object())
        if (json.contains("shader"))
        {
            auto shaderObj = json["shader"];

            std::string shaderName = shaderObj.value("name", "");
            std::string vertexPath = shaderObj.value("vertex", "");
            std::string fragmentPath = shaderObj.value("fragment", "");

            auto shaderProgram = LoadProgram(shaderName, vertexPath, fragmentPath);
            if (!shaderProgram)
            {
                return nullptr;
            }

            result = std::make_shared<Material>();
            result->SetShaderProgram(shaderProgram);
        }

        //  新增：2、着色器变体列表
        if (json.contains("variants") && json["variants"].is_array())
        {
            for (const auto &v : json["variants"])
            {
                const PassTag pass = PassTagFromString(v.value("pass", "forward"));

                std::string shaderName = v.value("name", "");
                std::string vertexPath = v.value("vertex", "");
                std::string fragmentPath = v.value("fragment", "");

                auto shaderProgram = LoadProgram(shaderName, vertexPath, fragmentPath);
                if (shaderProgram)
                {
                    result->SetVariantProgram(pass, shaderProgram);
                }
                else
                {
                    std::cerr << "[Material] Load: variant compile failed (pass=" << v.value("pass", "forward") << ") in " << path << std::endl;
                }
            }
        }

        if (json.contains("params"))
        {
            auto paramsObj = json["params"];

            //  浮点数
            if (paramsObj.contains("float"))
            {
                for (auto &p : paramsObj["float"])
                {
                    std::string name = p.value("name", "");
                    float v0 = p.value("v0", 0.0f);
                    result->SetFloat(name, v0);
                }
            }

            //  双浮点
            if (paramsObj.contains("float2"))
            {
                for (auto &p : paramsObj["float2"])
                {
                    std::string name = p.value("name", "");
                    float v0 = p.value("v0", 0.0f);
                    float v1 = p.value("v1", 0.0f);
                    result->SetVec2(name, glm::vec2(v0, v1));
                }
            }

            //  三浮点
            if (paramsObj.contains("float3"))
            {
                for (auto &p : paramsObj["float3"])
                {
                    std::string name = p.value("name", "");
                    float v0 = p.value("v0", 0.0f);
                    float v1 = p.value("v1", 0.0f);
                    float v2 = p.value("v2", 0.0f);
                    result->SetVec3(name, glm::vec3(v0, v1, v2));
                }
            }

            //  纹理
            if (paramsObj.contains("textures"))
            {
                for (auto &p : paramsObj["textures"])
                {
                    std::string name = p.value("name", "");
                    std::string texPath = p.value("path", "");

                    auto texture = Texture::Load(texPath);

                    result->SetTexture(name, texture);
                }
            }
        }
        return result;
    }
}