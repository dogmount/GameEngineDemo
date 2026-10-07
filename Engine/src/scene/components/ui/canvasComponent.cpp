#include "scene/components/ui/canvasComponent.h"
#include "scene/components/ui/UIElementComponent.h"
#include "scene/gameObject.h"
#include "shaderAPI/vertexLayout.h"
#include "render/mesh.h"
#include "engine/Engine.h"

namespace GL_engine
{
    void CanvasComponent::LoadProperties(const nlohmann::json &json)
    {
        bool active = json.value("active", true);
        SetActive(active);
    }

    void CanvasComponent::Update(float deltaTime)
    {
        if (!m_active)
        {
            return;
        }

        BeginRendering();
        const auto &children = m_owner->GetChildren();
        for (const auto &child : children)
        {
            if (auto comp = child->GetComponent<UIElementComponent>())
            {
                Render(comp);
            }
        }
        Flush();
    }

    void CanvasComponent::Init()
    {
        VertexLayout layout;
        //  position
        layout.elements.push_back({VertexElement::PositionIndex, 2, GL_FLOAT, 0});

        //  color
        layout.elements.push_back({VertexElement::ColorIndex, 4, GL_FLOAT, sizeof(float) * 2});

        //  uv
        layout.elements.push_back({VertexElement::UVIndex, 2, GL_FLOAT, sizeof(float) * 6});

        layout.stride = sizeof(float) * 8;

        m_mesh = std::make_shared<Mesh>(layout, m_vertices, m_indices);
    }

    void CanvasComponent::Render(UIElementComponent *element)
    {
        if (!element)
        {
            return;
        }

        element->Render(this);

        const auto &children = element->GetOwner()->GetChildren();
        for (const auto &child : children)
        {
            if (auto comp = child->GetComponent<UIElementComponent>())
            {
                Render(comp);
            }
        }
    }

    //  开始绘制
    void CanvasComponent::BeginRendering()
    {
        m_vertices.clear();
        m_indices.clear();
        m_batches.clear();
    }

    //  刷新
    void CanvasComponent::Flush()
    {
        m_mesh->UpdateDynamic(m_vertices, m_indices);
        auto &shaderAPI = Engine::GetInstance().GetShaderAPI();
        const auto &viewport = shaderAPI.GetViewport();

        RenderCommandUI command;
        command.mesh = m_mesh.get();
        command.shaderProgram = shaderAPI.GetDefaultProgramUI().get();
        command.batches = m_batches;
        command.screenWidth = viewport.width;
        command.screenHeight = viewport.height;

        Engine::GetInstance().GetRenderQueue().SubmitRenderCommand(command);
    }

    //  收集UI
    void CanvasComponent::CollectUI(UIElementComponent *element, std::vector<UIElementComponent *> &out)
    {
        out.push_back(element);

        const auto &children = element->GetOwner()->GetChildren();
        for (const auto &child : children)
        {
            if (auto component = child->GetComponent<UIElementComponent>())
            {
                CollectUI(component, out);
            }
        }
    }

    void CanvasComponent::DrawRect(
        const glm::vec2 &p1, const glm::vec2 &p2,
        const glm::vec2 &uv1, const glm::vec2 &uv2,
        Texture *texture, const glm::vec4 &color)
    {
        uint32_t base = static_cast<uint32_t>(m_vertices.size() / 8);

        m_vertices.insert(m_vertices.end(),
                          {p2.x, p2.y, color.r, color.b, color.g, color.a, uv2.x, uv2.y,
                           p1.x, p2.y, color.r, color.b, color.g, color.a, uv1.x, uv2.y,
                           p1.x, p1.y, color.r, color.b, color.g, color.a, uv1.x, uv1.y,
                           p2.x, p1.y, color.r, color.b, color.g, color.a, uv2.x, uv1.y});
        m_indices.insert(m_indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});

        UpdateBatches(texture);
    }

    //  按钮UI绘制
    void CanvasComponent::DrawRect(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec4 &color)
    {
        uint32_t base = static_cast<uint32_t>(m_vertices.size() / 8);
        m_vertices.insert(m_vertices.end(),
                          {p2.x, p2.y, color.r, color.b, color.g, color.a, 1.0f, 1.0f,
                           p1.x, p2.y, color.r, color.b, color.g, color.a, 0.0f, 1.0f,
                           p1.x, p1.y, color.r, color.b, color.g, color.a, 0.0f, 0.0f,
                           p2.x, p1.y, color.r, color.b, color.g, color.a, 1.0f, 0.0f});
        m_indices.insert(m_indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});

        UpdateBatches(nullptr);
    }

    //  设置/获取画布活动状态
    void CanvasComponent::SetActive(bool active)
    {
        m_active = active;
    }
    bool CanvasComponent::IsActive() const
    {
        return m_active;
    }

    //  更新UI批次
    void CanvasComponent::UpdateBatches(Texture *texture)
    {
        if (m_batches.empty() || m_batches.back().texture != texture)
        {
            m_batches.push_back({texture, 6});
        }
        else
        {
            m_batches.back().indexCount += 6;
        }
    }
}
