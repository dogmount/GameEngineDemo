#ifndef GL_ENGINE_RENDER_PIPELINE_H
#define GL_ENGINE_RENDER_PIPELINE_H
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "render/renderCommon.h"
#include "render/renderQueue.h"
#include "render/sceneRenderer.h"
#include "shaderAPI/shaderAPI.h"
#include "shaderAPI/FBOManager.h"

namespace GL_engine
{
    class Mesh;
    class Texture;

    //  可自定义渲染步骤的渲染管线：
    //  负责 Pass 的构建、顺序搭配与渲染执行
    class RenderPipeline
    {
    public:
        RenderPipeline(ShaderAPI &shaderAPI, RenderQueue &renderQueue, SceneRenderer &sceneRenderer, FBOManager &frameBuffers);
        ~RenderPipeline() = default;

        RenderPipeline(const RenderPipeline &) = delete;
        RenderPipeline &operator=(const RenderPipeline &) = delete;

        // ---------- 管线配置 ----------
        int AddPass(const RenderPass &pass);               //  追加一个Pass，返回其索引
        int InsertPass(int index, const RenderPass &pass); //  指定位置插入Pass
        bool RemovePass(const std::string &name);          //  按名称移除Pass
        void ClearPasses();                                //  清空所有Pass
        size_t GetPassCount() const;

        void SetPassUniform(const std::string &passName, const std::string &uniform, RenderPassUniformValue value); //  给Pass设置动态uniform

        // ---------- 执行 ----------
        void Execute(const CameraData &camera, const std::vector<LightData> &lights, int screenWidth, int screenHeight);

        // ---------- 查询 ----------
        const std::shared_ptr<Texture> GetColorTexture(const std::string &name, int attachmentIndex = 0) const;

        const std::shared_ptr<Texture> GetColorTexture(const std::string &name, const std::string &attachmentName) const;

        const std::shared_ptr<Texture> GetDepthTexture(const std::string &name) const;

        // ---------- 窗口尺寸变化 ----------
        void Resize(int width, int height);

    private:
        FBO *EnsureFBOForPass(const RenderPass &pass);                                                            //  按 fboSpec 幂等创建/复用输出 FBO
        void ExecutePass(const RenderPass &pass, const CameraData &camera, const std::vector<LightData> &lights); //  单 Pass 执行流程

        void SetTargetState(FBO *target, int &outWidth, int &outHeight); //  绑定目标+视口

        void ClearTarget(FBO *target, const RenderPass &pass); //  清屏(FBO 或屏幕)
        void OverrideGLState(const RenderPass &pass);          //  openGL状态覆盖
        void DefaultGLState();                                 //  openGL默认状态

        void InputUniforms(ShaderProgram *shader, const RenderPass &pass, const CameraData &camera); //  设置pass的uniform值
        void InputTextures(ShaderProgram *shader, const RenderPass &pass);                           //  设置pass的texture
        void DrawFullscreenQuad(ShaderProgram *shader);                                              //  后处理全屏绘制

    private:
        ShaderAPI &m_shaderAPI;
        RenderQueue &m_renderQueue;
        SceneRenderer &m_sceneRenderer;
        FBOManager &m_frameBuffers;

        std::vector<RenderPass> m_passes;       //  按顺序执行的所有Pass
        std::shared_ptr<Mesh> m_fullscreenQuad; //  全屏四边形网格(后处理用)

        int m_screenWidth = 0;
        int m_screenHeight = 0;
    };
}

#endif