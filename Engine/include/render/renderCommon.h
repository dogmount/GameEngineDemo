#ifndef RENDER_COMMON_H
#define RENDER_COMMON_H
#pragma once

#include "shaderAPI/FBO.h"
#include "shaderAPI/shaderAPI.h"
#include "shaderAPI/shaderProgram.h"
#include "engine/common.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <variant>

namespace GL_engine
{
    class FBOManager;
    class SceneRenderer;
    class RenderQueue;

    //  ============================================================
    //  PassTag: 渲染阶段标签
    //  意图:
    //    1) 渲染器据此向材质索取"该阶段应使用的 shader 变体";
    //    2) 同时作为"输出附件布局"的约定标识
    //       (例如 GBuffer 阶段约定 0=albedo, 1=normal, 2=pbr)
    //  ============================================================
    enum class PassTag
    {
        Forward, // 主几何(默认): 完整光照着色
        Depth,   // 深度预写: 只需 position → depth
        Shadow,  // 阴影: 光源视角深度(可带 AlphaTest 变体)
        GBuffer, // 延迟: MRT 输出
        Custom   // 自定义阶段
    };

    //  Pass 类型
    enum class RenderPassType
    {
        Geometry,    //  3D Pass：绘制3D
        Sprite,      //  2D Pass：绘制2D
        PostProcess, //  后处理Pass：全屏四边形采样输入纹理
        UI,          //  UI Pass：绘制UI
        Blit,        //  Blit操作
        Resolve,     //  MSAA->单采样
        Custom,      //  自定义Pass：执行 custom 回调
    };

    //  输入绑定：FBO附件->uniform sampler2D
    struct RenderPassInput
    {
        std::string fbo;            //  输入FBO名称
        int attachmentIndex = 0;    //  输入FBO的颜色附件索引
        std::string attachmentName; //  附件语义名(优先于下标, MRT/延迟用)
        std::string uniform;        //  绑定到着色器的 uniform sampler2D 名称(空 = uInputTextureN)
        bool depth = false;         //  true = 取深度附件
    };

    //  Pass 级 uniform 值：覆盖常用类型(纹理不走这里, 走 inputBindings)
    using RenderPassUniformValue = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4>;

    //  输入绑定：uniform
    struct RenderPassUniform
    {
        std::string name;             //  Uniform名
        RenderPassUniformValue value; //  Uniform值

        template <typename T>
        RenderPassUniform(std::string n, T v) : name(std::move(n)), value(std::move(v)) {}
    };

    //  渲染Pass描述：一个可自定义的渲染步骤
    struct RenderPass
    {
        std::string name; //  Pass名称(调试标识)
        RenderPassType type = RenderPassType::Geometry;
        PassTag passTag = PassTag::Forward; // 意图: 几何类 Pass 的材质变体选择依据

        // ---------- 输出 ----------
        std::string outputFBO; //  输出FBO名称；空 = 渲染到默认帧缓冲(屏幕)
        FBOSpec fboSpec;       //  输出FBO规格(由FrameBuffer管理器按outputName创建)

        // ---------- 清屏 ----------
        bool clearColorBuffer = true;
        bool clearDepthBuffer = true;
        bool clearStencilBuffer = true;
        glm::vec4 clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        float clearDepth = 1.0f;
        int clearStencil = 0;

        // ---------- 后处理 ----------
        std::string shaderName;                     //  按名称从ShaderAPI取着色器
        std::vector<RenderPassInput> inputBindings; //  输入绑定：FBO附件->uniform sampler2D

        //  ---------- Pass 级 uniform ----------
        std::vector<RenderPassUniform> uniforms; //  意图: 标量/向量/矩阵参数(曝光、阈值、方向、调试模式...)
        bool bindCameraUniforms = false;

        // ---------- Resolve Pass ----------
        std::string sourceFBO;                        //  源 FBO 名
        GLbitfield resolveMask = GL_COLOR_BUFFER_BIT; //  默认只解析颜色
        GLenum resolveFilter = GL_LINEAR;

        // ---------- 渲染状态覆盖 ----------
        int depthTest = -1;         //  -1=保持默认, 0=关闭, 1=开启
        bool overrideBlend = false; //  是否覆盖混合模式
        BlendMode blendMode = BlendMode::Disabled;

        // ---------- 自定义 ----------
        //  自定义Pass的绘制逻辑(可访问ShaderAPI、FrameBuffer管理器、相机与灯光)
        std::function<void(ShaderAPI &, FBOManager &, SceneRenderer &, const CameraData &, const std::vector<LightData> &)> custom;
    };
}

#endif