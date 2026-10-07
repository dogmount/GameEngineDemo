#ifndef RENDER_QUEUE_H
#define RENDER_QUEUE_H
#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "engine/common.h"

namespace GL_engine
{
    class Mesh;
    class Material;
    class Texture;
    class ShaderAPI;
    class ShaderProgram;

    //  渲染指令
    struct RenderCommand
    {
        //  网格指针
        Mesh *mesh = nullptr;

        //  材质指针
        Material *material = nullptr;

        //  模型矩阵
        glm::mat4 modelMatrix;
    };

    //  2D渲染指令
    struct RenderCommand2D
    {
        glm::mat4 modelMatrix; //  模型矩阵
        Texture *texture = nullptr;
        glm::vec4 color;
        glm::vec2 size;
        glm::vec2 lowerLeftUV;
        glm::vec2 upperRightUV;
        glm::vec2 pivot;
    };

    //  UI渲染指令
    struct RenderCommandUI
    {
        Mesh *mesh;
        ShaderProgram *shaderProgram;
        size_t screenWidth;
        size_t screenHeight;
        std::vector<UIBatch> batches;
    };

    class RenderQueue
    {
    public:
        void Init();
        //  提交3D渲染指令
        void SubmitRenderCommand(const RenderCommand &command);

        //  提交2D渲染指令
        void SubmitRenderCommand(const RenderCommand2D &command);

        //  提交UI渲染指令
        void SubmitRenderCommand(const RenderCommandUI &command);

        //  ---------- 只读遍历(绘制器 / 剔除 / 排序 / 调试) ----------
        const std::vector<RenderCommand> &GetCommands3D() const;
        const std::vector<RenderCommand2D> &GetCommands2D() const;
        const std::vector<RenderCommandUI> &GetCommandsUI() const;

        //  ---------- 统计 ----------
        size_t GetCommandCount3D() const;
        size_t GetCommandCount2D() const;
        size_t GetCommandCountUI() const;

        //  ---------- 清空(帧末由管线统一调用) ----------
        void ClearAll();

        //  【保留】正向渲染，绘制所有指令(3D+2D+UI)
        // void Draw(ShaderAPI &shaderAPI, const CameraData &cameraData, const std::vector<LightData> &lights);

    private:
        //  3D渲染指令队列
        std::vector<RenderCommand> m_commands;

        //  2D渲染指令队列
        std::vector<RenderCommand2D> m_commands2D;

        //  UI渲染指令队列
        std::vector<RenderCommandUI> m_commandsUI;

        //  2D网格
        // std::shared_ptr<Mesh> m_mesh2D;//  【保留】正向渲染
    };
}

#endif