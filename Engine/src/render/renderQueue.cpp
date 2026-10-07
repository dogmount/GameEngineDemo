#include "render/renderQueue.h"
#include "render/mesh.h"
#include "render/material.h"
#include "shaderAPI/texture.h"
#include "shaderAPI/shaderAPI.h"
#include "shaderAPI/shaderProgram.h"

#include <glm/gtc/matrix_transform.hpp>

namespace GL_engine
{
    void RenderQueue::Init()
    {
        // m_mesh2D = Mesh::CreatePlane2D();//  原正向渲染（保留）
    }

    //  提交3D渲染指令
    void RenderQueue::SubmitRenderCommand(const RenderCommand &command)
    {
        m_commands.push_back(command);
    }

    //  提交2D渲染指令
    void RenderQueue::SubmitRenderCommand(const RenderCommand2D &command)
    {
        m_commands2D.push_back(command);
    }

    //  提交UI渲染指令
    void RenderQueue::SubmitRenderCommand(const RenderCommandUI &command)
    {
        m_commandsUI.push_back(command);
    }

    //  ==================== 只读遍历 ====================
    const std::vector<RenderCommand> &RenderQueue::GetCommands3D() const
    {
        return m_commands;
    }

    const std::vector<RenderCommand2D> &RenderQueue::GetCommands2D() const
    {
        return m_commands2D;
    }

    const std::vector<RenderCommandUI> &RenderQueue::GetCommandsUI() const
    {
        return m_commandsUI;
    }

    //  ==================== 统计 ====================
    size_t RenderQueue::GetCommandCount3D() const
    {
        return m_commands.size();
    }
    size_t RenderQueue::GetCommandCount2D() const
    {
        return m_commands2D.size();
    }
    size_t RenderQueue::GetCommandCountUI() const
    {
        return m_commandsUI.size();
    }

    //  ==================== 清空 ====================
    void RenderQueue::ClearAll()
    {
        m_commands.clear();
        m_commands2D.clear();
        m_commandsUI.clear();
    }

#if 0
    //  执行渲染队列中命令
    void RenderQueue::Draw(ShaderAPI &shaderAPI, const CameraData &cameraData, const std::vector<LightData> &lights)
    {
        //  3D
        for (auto &command : m_commands)
        {
            shaderAPI.BindMaterial(command.material);                  //  绑定材质
            auto shaderProgram = command.material->GetShaderProgram(); //  获取着色器程序

            shaderProgram->SetUniform("uModel", command.modelMatrix);              //  设置模型矩阵
            shaderProgram->SetUniform("uView", cameraData.viewMatrix);             //  设置视图矩阵
            shaderProgram->SetUniform("uProjection", cameraData.projectionMatrix); //  设置透视矩阵
            shaderProgram->SetUniform("uCameraPos", cameraData.position);          //  设置相机位置

            if (!lights.empty())
            {
                auto &light = lights[0];
                shaderProgram->SetUniform("uLight.color", light.color);
                shaderProgram->SetUniform("uLight.direction", glm::normalize(-light.position));
            }

            shaderAPI.BindMesh(command.mesh);   //  绑定网格
            shaderAPI.DrawMesh(command.mesh);   //  网格绘制
            shaderAPI.UnbindMesh(command.mesh); //  网格解绑
        }
        m_commands.clear();

        //  2D
        shaderAPI.SetDepthTestEnabled(false);     //  关闭深度测试
        shaderAPI.SetBlendMode(BlendMode::Alpha); //  设置Alpha混合模式

        const auto shaderProgram2D = shaderAPI.GetDefaultProgram2D();
        shaderProgram2D->ShaderBind();
        m_mesh2D->MeshBind();
        for (auto &command : m_commands2D)
        {
            //  rendering
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
        shaderAPI.SetBlendMode(BlendMode::Disabled); //  关闭混合模式
        shaderAPI.SetDepthTestEnabled(true);         //  重启深度测试
        m_commands2D.clear();

        //  UI
        shaderAPI.SetDepthTestEnabled(false);     //  关闭深度测试
        shaderAPI.SetBlendMode(BlendMode::Alpha); //  设置Alpha混合模式
        for (auto &command : m_commandsUI)
        {
            glm::mat4 ortho = glm::ortho(
                0.0f, static_cast<float>(command.screenWidth),
                0.0f, static_cast<float>(command.screenHeight));
            command.shaderProgram->ShaderBind();
            command.shaderProgram->SetUniform("uProjection", ortho);

            command.mesh->MeshBind();

            uint32_t indexBase = 0;
            for (auto &batch : command.batches)
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
        shaderAPI.SetBlendMode(BlendMode::Disabled); //  关闭混合模式
        shaderAPI.SetDepthTestEnabled(true);         //  重启深度测试
        m_commandsUI.clear();
    }
#endif
}