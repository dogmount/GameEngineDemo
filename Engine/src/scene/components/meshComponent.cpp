#include "scene/components/meshComponent.h"
#include "render/mesh.h"
#include "render/material.h"
#include "render/renderQueue.h"
#include "scene/gameObject.h"
#include "engine/Engine.h"

#include <nlohmann/json.hpp>

namespace GL_engine
{
    MeshComponent::MeshComponent(const std::shared_ptr<Material> &material, const std::shared_ptr<Mesh> &mesh)
        : m_material(material), m_mesh(mesh)
    {
    }

    //  加载属性
    void MeshComponent::LoadProperties(const nlohmann::json &json)
    {
        if (json.contains("material"))
        {
            auto &matObj = json["material"];
            const std::string path = matObj.value("path", "");
            auto mat = Material::Load(path);
            if (mat && matObj.contains("params"))
            {
                auto &paramsObj = matObj["params"];

                //  浮点数
                if (paramsObj.contains("float"))
                {
                    for (auto &p : paramsObj["float"])
                    {
                        std::string name = p.value("name", "");
                        float value = p.value("v0", 0.0f);
                        mat->SetFloat(name, value);
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
                        mat->SetVec2(name, glm::vec2(v0, v1));
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
                        mat->SetVec3(name, glm::vec3(v0, v1, v2));
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

                        mat->SetTexture(name, texture);
                    }
                }
            }
            SetMaterial(mat);
        }

        if (json.contains("mesh"))
        {
            const auto &meshObj = json["mesh"];
            const std::string type = meshObj.value("type", "box");
            if (type == "box")
            {
                float w=meshObj.value("width", 1.0f);
                float h=meshObj.value("height", 1.0f);
                float d=meshObj.value("depth", 1.0f);
                float wSeg=meshObj.value("widthSegments", 1.0f);
                float hSeg=meshObj.value("heightSegments", 1.0f);
                float dSeg=meshObj.value("depthSegments", 1.0f);
                auto mesh = Mesh::CreateBox(w, h, d, wSeg, hSeg, dSeg);
                SetMesh(mesh);
            }
            else if (type == "sphere")
            {
                float r = meshObj.value("r", 1.0f);
                int wSegments=meshObj.value("widthSegments", 16);
                int hSegments=meshObj.value("heightSegments", 16);
                auto mesh = Mesh::CreateSphere(r, wSegments, hSegments);
                SetMesh(mesh);
            }
            else if(type=="prism")
            {
                //  读取 3 个底面点 + 挤出向量(局部坐标); 缺省给一个沿 X 挤出的楔形
                auto readVec3 = [&](const char *key, const glm::vec3 &def) -> glm::vec3
                {
                    if (!meshObj.contains(key) || !meshObj[key].is_array() || meshObj[key].size() != 3)
                    {
                        return def;
                    }
                    return glm::vec3(
                        meshObj[key][0].get<float>(),
                        meshObj[key][1].get<float>(),
                        meshObj[key][2].get<float>());
                };

                const glm::vec3 a = readVec3("a", glm::vec3(0.0f, 0.0f, -0.5f));
                const glm::vec3 b = readVec3("b", glm::vec3(0.0f, 0.0f, 0.5f));
                const glm::vec3 c = readVec3("c", glm::vec3(0.0f, 1.0f, 0.5f));
                const glm::vec3 extrude = readVec3("extrude", glm::vec3(1.0f, 0.0f, 0.0f));

                auto mesh = Mesh::CreatePrism(a, b, c, extrude);
                SetMesh(mesh);
            }
        }
    }

    void MeshComponent::Update(float deltaTime)
    {
        if (!m_material || !m_mesh)
        {
            return;
        }

        //  提供渲染命令
        GL_engine::RenderCommand command;
        command.material = m_material.get();
        command.mesh = m_mesh.get();
        command.modelMatrix = GetOwner()->GetWorldTransform();

        //  提交渲染命令
        auto &renderQueue = GL_engine::Engine::GetInstance().GetRenderQueue();
        renderQueue.SubmitRenderCommand(command);
    }

    //  设置材质/网格
    void MeshComponent::SetMaterial(const std::shared_ptr<Material> &material)
    {
        m_material = material;
    }
    void MeshComponent::SetMesh(const std::shared_ptr<Mesh> &mesh)
    {
        m_mesh = mesh;
    }
}