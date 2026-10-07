#include "render/renderPipeline.h"
#include "render/mesh.h"
#include "shaderAPI/texture.h"
#include "shaderAPI/shaderProgram.h"

#include "glad/glad.h"

#include <iostream>

namespace GL_engine
{
    RenderPipeline::RenderPipeline(ShaderAPI &shaderAPI, RenderQueue &renderQueue, SceneRenderer &sceneRenderer, FBOManager &frameBuffers)
        : m_shaderAPI(shaderAPI), m_renderQueue(renderQueue), m_sceneRenderer(sceneRenderer), m_frameBuffers(frameBuffers)
    {
        m_fullscreenQuad = Mesh::FullscreenQuad();
    }

    // ---------- 追加Pass ----------
    int RenderPipeline::AddPass(const RenderPass &pass)
    {
        m_passes.push_back(pass);
        return static_cast<int>(m_passes.size()) - 1;
    }

    //  指定位置插入Pass
    int RenderPipeline::InsertPass(int index, const RenderPass &pass)
    {
        if (index < 0)
        {
            index = 0;
        }
        if (index > m_passes.size())
        {
            index = m_passes.size();
        }
        m_passes.insert(m_passes.begin() + index, pass);
        return static_cast<int>(index);
    }

    // ---------- 按名称移除Pass ----------
    bool RenderPipeline::RemovePass(const std::string &name)
    {
        for (auto it = m_passes.begin(); it != m_passes.end(); ++it)
        {
            if (it->name == name)
            {
                m_passes.erase(it);
                return true;
            }
        }
        return false;
    }

    // ---------- 清空所有Pass ----------
    void RenderPipeline::ClearPasses()
    {
        m_passes.clear();
    }

    // ---------- Pass数量 ----------
    size_t RenderPipeline::GetPassCount() const
    {
        return m_passes.size();
    }

    // ---------- Pass设置动态uniform ----------
    void RenderPipeline::SetPassUniform(const std::string &passName, const std::string &uniform, RenderPassUniformValue value)
    {
        for (auto &pass : m_passes)
        {
            if (pass.name != passName)
            {
                continue;
            }

            //  1) 已存在同名 uniform → 直接替换(类型也一起换)
            for (auto &u : pass.uniforms)
            {
                if (u.name == uniform)
                {
                    u.value = std::move(value);
                    return;
                }
            }

            //  2) 不存在 → 追加一条
            pass.uniforms.emplace_back(uniform, std::move(value));
            return;
        }

        std::cerr << "[RenderPipeline] SetPassUniform: pass \"" << passName << "\" not found (uniform \"" << uniform << "\" ignored)\n";
    }

    // ---------- 查询Pass输出纹理 ----------
    const std::shared_ptr<Texture> RenderPipeline::GetColorTexture(const std::string &name, int attachmentIndex) const
    {
        return m_frameBuffers.GetColorAttachment(name, attachmentIndex);
    }

    const std::shared_ptr<Texture> RenderPipeline::GetColorTexture(const std::string &name, const std::string &attachmentName) const
    {
        return m_frameBuffers.GetColorAttachment(name, attachmentName);
    }

    const std::shared_ptr<Texture> RenderPipeline::GetDepthTexture(const std::string &name) const
    {
        return m_frameBuffers.GetDepthAttachment(name);
    }

    // ---------- 窗口尺寸变化 ----------
    void RenderPipeline::Resize(int width, int height)
    {
        m_screenWidth = width;
        m_screenHeight = height;
        m_frameBuffers.Resize(width, height);
    }

    // ---------- 确保Pass输出FBO已创建 ----------
    FBO *RenderPipeline::EnsureFBOForPass(const RenderPass &pass)
    {
        if (pass.outputFBO.empty())
        {
            return nullptr;
        }

        FBO *fbo = m_frameBuffers.Create(pass.outputFBO, pass.fboSpec, m_screenWidth, m_screenHeight);
        if (!fbo || !fbo->IsValid())
        {
            std::cerr << "[RenderPipeline] Pass '" << pass.name << "' FBO \"" << pass.outputFBO << "\" unavailable\n";
            return nullptr;
        }

        return fbo;
    }

    // ---------- 执行整条管线 ----------
    void RenderPipeline::Execute(const CameraData &camera, const std::vector<LightData> &lights, int screenWidth, int screenHeight)
    {
        m_screenWidth = screenWidth;
        m_screenHeight = screenHeight;

        m_sceneRenderer.ClearGlobals(); // 清空上一帧的全局纹理/矩阵缓存

        for (auto &pass : m_passes)
        {
            ExecutePass(pass, camera, lights);
        }

        //  所有 Pass 已按需读取同一份命令数据, 帧末统一清理
        m_renderQueue.ClearAll();
    }

    // ---------- 执行单个Pass ----------
    void RenderPipeline::ExecutePass(const RenderPass &pass, const CameraData &camera, const std::vector<LightData> &lights)
    {
        // ---------- Blit / Resolve: 纯数据搬运, 无需绑定绘制目标 ----------
        if (pass.type == RenderPassType::Blit || pass.type == RenderPassType::Resolve)
        {
            if (pass.sourceFBO.empty())
            {
                std::cerr << "[RenderPipeline] Pass '" << pass.name << "' has no source FBO\n";
                return;
            }

            // 目标 FBO(单采样)需要先建出来; 源 FBO 由名字在管理器里查找
            if (!pass.outputFBO.empty() && !EnsureFBOForPass(pass))
            {
                return;
            }

            const bool toScreen = pass.outputFBO.empty();
            bool ok = false;

            if (pass.type == RenderPassType::Blit)
            {
                // 通用拷贝(不做 MSAA 语义判断)
                ok = toScreen
                         ? m_frameBuffers.BlitToScreen(pass.sourceFBO, m_screenWidth, m_screenHeight, pass.resolveMask, pass.resolveFilter)
                         : m_frameBuffers.Blit(pass.sourceFBO, pass.outputFBO, pass.resolveMask, pass.resolveFilter);
            }
            else
            {
                // MSAA 闭环(管理器内部按采样数分流)
                ok = toScreen
                         ? m_frameBuffers.ResolveToScreen(pass.sourceFBO, m_screenWidth, m_screenHeight, pass.resolveMask, pass.resolveFilter)
                         : m_frameBuffers.Resolve(pass.sourceFBO, pass.outputFBO, pass.resolveMask, pass.resolveFilter);
            }

            if (!ok)
            {
                std::cerr << "[RenderPipeline] Pass '" << pass.name << "' blit/resolve failed (" << pass.sourceFBO << " -> " << (toScreen ? "screen" : pass.outputFBO) << ")\n";
            }
            return;
        }

        // ---------- 常规 Pass: 1) 目标 ----------
        FBO *target = EnsureFBOForPass(pass);
        if (!pass.outputFBO.empty() && target == nullptr)
        {
            return; // 需要 FBO 但建不出来 → 跳过该 Pass
        }

        int vpW = 0;
        int vpH = 0;
        SetTargetState(target, vpW, vpH);

        // 2) 清屏
        ClearTarget(target, pass);

        // 3) Pass 级状态覆盖
        OverrideGLState(pass);

        // 4) 按类型分派
        switch (pass.type)
        {
        case RenderPassType::Geometry:
        {
            // shaderName 在几何 Pass 中充当"覆盖着色器"(调试视图/特殊 Pass);
            // 为空时由 SceneRenderer 按 (passTag, 材质特性) 从材质变体里选程序
            std::shared_ptr<ShaderProgram> overrideShader;
            if (!pass.shaderName.empty())
            {
                overrideShader = m_shaderAPI.GetShader(pass.shaderName);
                if (!overrideShader)
                {
                    std::cerr << "[RenderPipeline] pass '" << pass.name << "' shader \"" << pass.shaderName << "\" not found, 退化为材质shader!\n";
                }
            }

            //  ---------- 阴影投射光: 数据由 Scene::CollectLights() 生产, 管线只挑选 ----------
            const LightData *shadowCaster = nullptr;
            for (const auto &light : lights)
            {
                if (light.castsShadow)
                {
                    shadowCaster = &light;
                    break;
                }
            }

            //  ---------- 相机: Shadow Pass 换成"光视角正交相机"(来自 LightData::shadow) ----------
            const CameraData *useCamera = &camera;
            if (pass.passTag == PassTag::Shadow && shadowCaster)
            {
                useCamera = &shadowCaster->shadow.cameraData;
            }

            if (pass.passTag == PassTag::Forward)
            {
                if (auto shadowTex = m_frameBuffers.GetDepthAttachment("shadowMap"))
                {
                    m_sceneRenderer.SetGlobalTexture("uShadowMap", shadowTex);
                    if (shadowCaster)
                    {
                        m_sceneRenderer.SetGlobalMat4("uLightViewProj", shadowCaster->shadow.viewProj);
                    }
                }
                //  新增: 模糊后的 AO(SSAOBlur 在同一帧更早执行, 纹理已就绪)
                if (auto aoTex = m_frameBuffers.GetColorAttachment("ssaoBlur", 0))
                {
                    m_sceneRenderer.SetGlobalTexture("uSSAO", aoTex);
                }
            }

            //  阴影 Pass: 多边形偏移, 把写入阴影图的深度整体推远
            //  → 治自遮蔽(acne); 对所有材质生效, 不依赖各 shader 自己的 bias
            if (pass.passTag == PassTag::Shadow)
            {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(2.0f, 4.0f); // 太大会让接触阴影脱开, 2/4 是通用起点
            }

            m_sceneRenderer.BeginPass(); // 清空上一 Pass 的 材质/程序 绑定缓存
            m_sceneRenderer.Draw3D(m_shaderAPI, m_renderQueue.GetCommands3D(), *useCamera, lights, pass.passTag, overrideShader.get());

            if (pass.passTag == PassTag::Shadow)
            {
                glDisable(GL_POLYGON_OFFSET_FILL);
            }

            break;
        }

        case RenderPassType::Sprite:
            m_sceneRenderer.BeginPass();
            m_sceneRenderer.Draw2D(m_shaderAPI, m_renderQueue.GetCommands2D(), camera);
            break;

        case RenderPassType::UI:
            m_sceneRenderer.BeginPass();
            m_sceneRenderer.DrawUI(m_shaderAPI, m_renderQueue.GetCommandsUI());
            break;

        case RenderPassType::PostProcess:
        {
            std::shared_ptr<ShaderProgram> shader;
            if (!pass.shaderName.empty())
            {
                shader = m_shaderAPI.GetShader(pass.shaderName);
            }

            if (shader)
            {
                shader->ShaderBind();
                InputTextures(shader.get(), pass);         //  输入pass所需texture
                InputUniforms(shader.get(), pass, camera); //  输入pass所需uniform
                DrawFullscreenQuad(shader.get());
            }
            else
            {
                std::cerr << "[RenderPipeline] PostProcess '" << pass.name << "' has no shader (\"" << pass.shaderName << "\")\n";
            }
            break;
        }

        case RenderPassType::Custom:
            if (pass.custom)
            {
                pass.custom(m_shaderAPI, m_frameBuffers, m_sceneRenderer, camera, lights);
            }
            break;

        default:
            break;
        }

        // 5) 状态复位(避免 Pass 间状态泄漏)
        DefaultGLState();

        // 3) 解绑目标
        if (target)
        {
            target->FBOUnbind();
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    // ---------- 绑定目标 + 视口 ----------
    void RenderPipeline::SetTargetState(FBO *target, int &outWidth, int &outHeight)
    {
        if (target)
        {
            target->FBOBind();
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        outWidth = target ? target->GetWidth() : m_screenWidth;
        outHeight = target ? target->GetHeight() : m_screenHeight;

        m_shaderAPI.SetViewport(0, 0, outWidth, outHeight);
    }

    // ---------- 清屏 ----------
    void RenderPipeline::ClearTarget(FBO *target, const RenderPass &pass)
    {
        if (!pass.clearColorBuffer && !pass.clearDepthBuffer && !pass.clearStencilBuffer)
        {
            return;
        }

        if (target)
        {
            if (pass.clearColorBuffer)
            {
                target->ClearColor(pass.clearColor);
            }
            if (pass.clearDepthBuffer)
            {
                target->ClearDepth(pass.clearDepth);
            }
            if (pass.clearStencilBuffer)
            {
                target->ClearStencil(pass.clearStencil);
            }
        }
        else
        {
            GLbitfield mask = 0;
            if (pass.clearColorBuffer)
            {
                const GLfloat color[4] = {
                    pass.clearColor.r, pass.clearColor.g,
                    pass.clearColor.b, pass.clearColor.a};
                glClearBufferfv(GL_COLOR, 0, color);
            }
            if (pass.clearDepthBuffer)
            {
                glClearBufferfv(GL_DEPTH, 0, &pass.clearDepth);
            }
            if (pass.clearStencilBuffer)
            {
                glClearBufferiv(GL_STENCIL, 0, &pass.clearStencil);
            }
        }
    }

    // ---------- Pass 状态 ----------
    void RenderPipeline::OverrideGLState(const RenderPass &pass)
    {
        if (pass.depthTest == 0)
        {
            m_shaderAPI.SetDepthTestEnabled(false);
        }
        else if (pass.depthTest == 1)
        {
            m_shaderAPI.SetDepthTestEnabled(true);
        }

        if (pass.overrideBlend)
        {
            m_shaderAPI.SetBlendMode(pass.blendMode);
        }
    }

    void RenderPipeline::DefaultGLState()
    {
        // 意图: 统一恢复默认, 避免 Pass 间状态泄漏
        m_shaderAPI.SetDepthTestEnabled(true);
        m_shaderAPI.SetBlendMode(BlendMode::Disabled);
    }

    // ---------- 输入数据 ----------
    //  设置pass的uniform值
    void RenderPipeline::InputUniforms(ShaderProgram *shader, const RenderPass &pass, const CameraData &camera)
    {
        if (!shader)
        {
            return;
        }

        //  1) 标准相机 uniform(按需注入, 免去每个 shader 都要管线手动设)
        if (pass.bindCameraUniforms)
        {
            shader->SetUniform("uView", camera.viewMatrix);
            shader->SetUniform("uProjection", camera.projectionMatrix);
            shader->SetUniform("uCameraPos", camera.position);
            shader->SetUniform("uScreenSize", glm::vec2(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight)));
            //  注: 需要逆投影时, 在 shader 里 `inverse(uProjection)` 即可, 不必再传
        }

        //  2) Pass 自定义 uniform
        for (const auto &u : pass.uniforms)
        {
            std::visit(
                [&](const auto &v)
                {
                    using T = std::decay_t<decltype(v)>;//  decltype(v) 是v的声明类型，参数是 const auto& v，decltype(v) 是 const T&；std::decay_t 去掉const和&，得到纯类型T
                    if constexpr (std::is_same_v<T, int>)
                    {
                        shader->SetUniform(u.name, v);
                    }
                    else if constexpr (std::is_same_v<T, float>)
                    {
                        shader->SetUniform(u.name, v);
                    }
                    else if constexpr (std::is_same_v<T, glm::vec2>)
                    {
                        shader->SetUniform(u.name, v);
                    }
                    else if constexpr (std::is_same_v<T, glm::vec3>)
                    {
                        shader->SetUniform(u.name, v);
                    }
                    else if constexpr (std::is_same_v<T, glm::vec4>)
                    {
                        shader->SetUniform(u.name, v);
                    }
                    else if constexpr (std::is_same_v<T, glm::mat4>)
                    {
                        shader->SetUniform(u.name, v);
                    } },
                u.value);
        }
    }

    //  纹理绑定
    void RenderPipeline::InputTextures(ShaderProgram *shader, const RenderPass &pass)
    {
        if (!shader)
        {
            return;
        }

        int slot = 0;

        for (const auto &input : pass.inputBindings)
        {
            std::shared_ptr<Texture> tex;

            if (input.depth)
            {
                tex = m_frameBuffers.GetDepthAttachment(input.fbo);
            }
            else if (!input.attachmentName.empty())
            {
                tex = m_frameBuffers.GetColorAttachment(input.fbo, input.attachmentName);
            }
            else
            {
                tex = m_frameBuffers.GetColorAttachment(input.fbo, input.attachmentIndex);
            }

            if (!tex)
            {
                std::cerr << "[RenderPipeline] Pass '" << pass.name << "' input \"" << input.fbo << "\" missing\n";
                ++slot;
                continue;
            }

            const std::string uniform = input.uniform.empty() ? ("uInputTexture" + std::to_string(slot)) : input.uniform;
            shader->SetTexture(uniform, tex.get());
            ++slot;
        }
    }

    // ---------- 全屏四边形 ----------
    void RenderPipeline::DrawFullscreenQuad(ShaderProgram *shader)
    {
        if (!m_fullscreenQuad || !shader)
        {
            return;
        }

        m_fullscreenQuad->MeshBind();
        m_fullscreenQuad->MeshDraw();
        m_fullscreenQuad->MeshUnbind();
    }
}