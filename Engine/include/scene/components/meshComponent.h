#ifndef MESH_COMPONENT_H
#define MESH_COMPONENT_H
#pragma once

#include <memory>

#include "scene/component.h"

namespace GL_engine
{
    class Mesh;
    class Material;

    class MeshComponent : public Component
    {
        COMPONENT(MeshComponent)
    public:
        MeshComponent() = default;
        MeshComponent(const std::shared_ptr<Material> &material, const std::shared_ptr<Mesh> &mesh);

        //  加载属性
        void LoadProperties(const nlohmann::json &json) override;
        void Update(float deltaTime) override;

        //  设置材质/网格
        void SetMaterial(const std::shared_ptr<Material> &material);
        void SetMesh(const std::shared_ptr<Mesh> &mesh);

    private:
        std::shared_ptr<Material> m_material;
        std::shared_ptr<Mesh> m_mesh;
    };
}

#endif