#ifndef SCENE_RENDERER_H
#define SCENE_RENDERER_H
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "engine/common.h"
#include "render/renderCommon.h"
#include "render/renderQueue.h"

namespace GL_engine
{
    class Mesh;
    class Material;
    class ShaderAPI;
    class ShaderProgram;

    //  ============================================================
    //  SceneRenderer(绘制执行器): 只负责"怎么把命令画出来"
    //    - 不知道画到哪张 FBO(FBOManager 管) / 第几个 Pass(RenderPipeline 管)
    //    - 统一封装 3D/2D/UI 的材质绑定、uniform 约定与绘制状态
    //  ============================================================
    class SceneRenderer
    {
    public:
        void Init(); //  创建内部 2D 网格(原属 RenderQueue)

        //  每个 Pass 开始前调用, 清空"材质/程序绑定缓存"
        void BeginPass();

        //  3D: overrideShader != nullptr 时替换材质着色器(阴影深度 Pass 等)
        void Draw3D(ShaderAPI &shaderAPI, const std::vector<RenderCommand> &commands, const CameraData &cameraData, const std::vector<LightData> &lights, PassTag passTag = PassTag::Forward, ShaderProgram *overrideShader = nullptr);

        //  2D 精灵
        void Draw2D(ShaderAPI &shaderAPI, const std::vector<RenderCommand2D> &commands, const CameraData &cameraData);

        //  UI
        void DrawUI(ShaderAPI &shaderAPI, const std::vector<RenderCommandUI> &commands);

        //  设置全局纹理/矩阵(供材质采样)
        void SetGlobalTexture(const std::string &name, const std::shared_ptr<Texture> &texture);
        void SetGlobalMat4(const std::string &name, const glm::mat4 &mat);
        void ClearGlobals();

    private:
        std::shared_ptr<Mesh> m_mesh2D; //  2D 平面网格(原所属 RenderQueue)

        //  绑定缓存——连续绘制同一 (材质+程序) 时跳过 glUseProgram 与材质参数上传
        Material *m_lastMaterial = nullptr;
        ShaderProgram *m_lastProgram = nullptr;

        std::unordered_map<std::string, std::shared_ptr<Texture>> m_GlobalTextures; //  全局纹理(供材质采样)
        std::unordered_map<std::string, glm::mat4> m_GlobalMat4;                    //  全局矩阵(供材质采样)
    };
}

#endif