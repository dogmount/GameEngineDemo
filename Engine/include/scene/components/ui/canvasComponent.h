#ifndef CANVANS_COMPONENT_H
#define CANVANS_COMPONENT_H
#pragma once

#include "engine/common.h"
#include "scene/component.h"

#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace GL_engine
{
    class Mesh;
    class Texture;
    class UIElementComponent;

    class CanvasComponent : public Component
    {
        COMPONENT(CanvasComponent)
    public:
        void LoadProperties(const nlohmann::json &json) override;

        void Update(float deltaTime) override;
        void Init() override;
        void Render(UIElementComponent *element);

        //  开始绘制
        void BeginRendering();

        //  刷新
        void Flush();

        //  收集UI
        void CollectUI(UIElementComponent *element, std::vector<UIElementComponent *> &out);

        //  文本UI绘制
        void DrawRect(
            const glm::vec2 &lowerLeftPos, const glm::vec2 &upperRightPos,
            const glm::vec2 &lowerLeftUV, const glm::vec2 &upperRightUV,
            Texture *texture, const glm::vec4 &color);

        //  按钮UI绘制
        void DrawRect(const glm::vec2 &lowerLeftPos, const glm::vec2 &upperRightPos, const glm::vec4 &color);

        //  设置/获取画布活动状态
        void SetActive(bool active);
        bool IsActive() const;

    private:
        //  更新UI批次
        void UpdateBatches(Texture *texture);

    private:
        std::vector<UIBatch> m_batches; //  UI批次
        std::vector<float> m_vertices;
        std::vector<uint32_t> m_indices;
        std::shared_ptr<Mesh> m_mesh;

        bool m_active = true; //  画布活动状态
    };
}

#endif