#include "render/sceneRenderer.h"
#include "render/mesh.h"
#include "render/material.h"
#include "shaderAPI/texture.h"
#include "shaderAPI/shaderAPI.h"
#include "shaderAPI/shaderProgram.h"

#include <glm/gtc/matrix_transform.hpp>

namespace GL_engine
{
    void SceneRenderer::Init()
    {
        m_mesh2D = Mesh::CreatePlane2D();
        BeginPass();
    }

    //  Pass 之间材质/程序可能完全不同, 缓存必须更新
    void SceneRenderer::BeginPass()
    {
        m_lastMaterial = nullptr;
        m_lastProgram = nullptr;
    }

    //  ==================== 3D ====================
    void SceneRenderer::Draw3D(ShaderAPI &shaderAPI, const std::vector<RenderCommand> &commands, const CameraData &cameraData, const std::vector<LightData> &lights, PassTag passTag, ShaderProgram *overrideShader)
    {
        //  ---- 常规: 按材质绘制 ----
        for (const auto &command : commands)
        {
            if (!command.material || !command.mesh)
            {
                continue;
            }

            //  1) 选程序: 覆盖优先; 否则按 PassTag获取着色器变体
            ShaderProgram *program = overrideShader ? overrideShader : command.material->GetVariantProgram(passTag);

            if (!program)
            {
                continue;
            }

            //  2) 材质+程序切换时才绑定并上传材质参数(减少状态切换)
            if (program != m_lastProgram || command.material != m_lastMaterial)
            {
                program->ShaderBind(); // 内部重置纹理单元计数
                if (!overrideShader)
                {
                    command.material->BindParams(program); // 只向本次使用的程序传参
                }

                //  全局注入: 所有材质程序都能拿到 shadowMap / 光矩阵(材质自己的纹理先绑, 全局的放最后)
                for (auto &tex : m_GlobalTextures)
                {
                    if (tex.second)
                    {
                        program->SetTexture(tex.first, tex.second.get());
                    }
                }
                for (auto &mat4 : m_GlobalMat4)
                {
                    program->SetUniform(mat4.first, mat4.second);
                }

                m_lastProgram = program;
                m_lastMaterial = command.material;
            }

            program->SetUniform("uModel", command.modelMatrix);
            program->SetUniform("uView", cameraData.viewMatrix);
            program->SetUniform("uProjection", cameraData.projectionMatrix);

            //  只有前向阶段才需要相机位置与光照(Depth/Shadow/GBuffer 不传, 省开销)
            if (passTag == PassTag::Forward && !overrideShader)
            {
                program->SetUniform("uCameraPos", cameraData.position);

                if (!lights.empty())
                {
                    const auto &light = lights[0];
                    program->SetUniform("uLight.color", light.color);
                    program->SetUniform("uLight.direction", glm::normalize(-light.position));
                    program->SetUniform("uLight.position", light.position);
                }
            }

            shaderAPI.BindMesh(command.mesh);
            shaderAPI.DrawMesh(command.mesh);
            shaderAPI.UnbindMesh(command.mesh);
        }
    }

    //  ==================== 2D ====================
    void SceneRenderer::Draw2D(ShaderAPI &shaderAPI, const std::vector<RenderCommand2D> &commands, const CameraData &cameraData)
    {
        if (!m_mesh2D || commands.empty())
        {
            return;
        }

        shaderAPI.SetDepthTestEnabled(false);
        shaderAPI.SetBlendMode(BlendMode::Alpha);

        const auto shaderProgram2D = shaderAPI.GetDefaultProgram2D();
        shaderProgram2D->ShaderBind();
        m_mesh2D->MeshBind();

        for (const auto &command : commands)
        {
            shaderProgram2D->SetUniform("uModel", command.modelMatrix);
            shaderProgram2D->SetUniform("uView", cameraData.viewMatrix);
            shaderProgram2D->SetUniform("uProjection", cameraData.orthoMatrix);
            shaderProgram2D->SetUniform("uSize", command.size.x, command.size.y);
            shaderProgram2D->SetUniform("uPivot", command.pivot.x, command.pivot.y);
            shaderProgram2D->SetUniform("uUVmin", command.lowerLeftUV.x, command.lowerLeftUV.y);
            shaderProgram2D->SetUniform("uUVmax", command.upperRightUV.x, command.upperRightUV.y);
            shaderProgram2D->SetUniform("uColor", command.color);
            shaderProgram2D->SetTexture("uTex", command.texture);

            m_mesh2D->MeshDraw();
        }

        m_mesh2D->MeshUnbind();

        shaderAPI.SetBlendMode(BlendMode::Disabled);
        shaderAPI.SetDepthTestEnabled(true);
    }

    //  ==================== UI ====================
    void SceneRenderer::DrawUI(ShaderAPI &shaderAPI, const std::vector<RenderCommandUI> &commands)
    {
        if (commands.empty())
        {
            return;
        }

        shaderAPI.SetDepthTestEnabled(false);
        shaderAPI.SetBlendMode(BlendMode::Alpha);

        for (const auto &command : commands)
        {
            if (!command.shaderProgram || !command.mesh)
            {
                continue;
            }

            const glm::mat4 ortho = glm::ortho(
                0.0f, static_cast<float>(command.screenWidth),
                0.0f, static_cast<float>(command.screenHeight));

            command.shaderProgram->ShaderBind();
            command.shaderProgram->SetUniform("uProjection", ortho);

            command.mesh->MeshBind();

            uint32_t indexBase = 0;
            for (const auto &batch : command.batches)
            {
                if (batch.texture)
                {
                    command.shaderProgram->SetUniform("uUserTexture", 1);
                    command.shaderProgram->SetTexture("uTex", batch.texture);
                }
                else
                {
                    command.shaderProgram->SetUniform("uUserTexture", 0);
                }

                command.mesh->DrawIndexedRange(indexBase, batch.indexCount);
                indexBase += batch.indexCount;
            }

            command.mesh->MeshUnbind();
        }

        shaderAPI.SetBlendMode(BlendMode::Disabled);
        shaderAPI.SetDepthTestEnabled(true);
    }

    void SceneRenderer::SetGlobalTexture(const std::string &name, const std::shared_ptr<Texture> &texture)
    {
        for (auto &tex : m_GlobalTextures)
        {
            if (tex.first == name)
            {
                tex.second = texture;
                return;
            }
        }
        m_GlobalTextures.emplace(name, texture);
    }
    void SceneRenderer::SetGlobalMat4(const std::string &name, const glm::mat4 &mat)
    {
        for (auto &mat4 : m_GlobalMat4)
        {
            if (mat4.first == name)
            {
                mat4.second = mat;
                return;
            }
        }
        m_GlobalMat4.emplace(name, mat);
    }
    void SceneRenderer::ClearGlobals()
    {
        m_GlobalTextures.clear();
        m_GlobalMat4.clear();
    }
}