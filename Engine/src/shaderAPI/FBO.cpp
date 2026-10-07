#include "shaderAPI/FBO.h"
#include "shaderAPI/texture.h"

#include <iostream>

#include <glm/gtc/type_ptr.hpp>

namespace GL_engine
{
    // ==================== 内部辅助 ====================

    //  根据颜色内部格式推导 glTexImage2D 所需的 format/type
    void GetColorFormatAndType(GLenum internalFormat, GLenum &outFormat, GLenum &outType)
    {
        switch (internalFormat)
        {
            // --- 8位归一化格式 ---
        case GL_R8:
            outFormat = GL_RED;
            outType = GL_UNSIGNED_BYTE;
            break;
        case GL_RG8:
            outFormat = GL_RG;
            outType = GL_UNSIGNED_BYTE;
            break;
        case GL_RGB8:
            outFormat = GL_RGB;
            outType = GL_UNSIGNED_BYTE;
            break;
        case GL_RGBA8:
            outFormat = GL_RGBA;
            outType = GL_UNSIGNED_BYTE;
            break;

        // --- 16位浮点格式 (type 使用 GL_HALF_FLOAT 或 GL_FLOAT 均可) ---
        case GL_R16F:
            outFormat = GL_RED;
            outType = GL_FLOAT;
            break;
        case GL_RG16F:
            outFormat = GL_RG;
            outType = GL_FLOAT;
            break;
        case GL_RGB16F:
            outFormat = GL_RGB;
            outType = GL_FLOAT;
            break;
        case GL_RGBA16F:
            outFormat = GL_RGBA;
            outType = GL_FLOAT;
            break;

        // --- 32位浮点格式 ---
        case GL_R32F:
            outFormat = GL_RED;
            outType = GL_FLOAT;
            break;
        case GL_RG32F:
            outFormat = GL_RG;
            outType = GL_FLOAT;
            break;
        case GL_RGB32F:
            outFormat = GL_RGB;
            outType = GL_FLOAT;
            break;
        case GL_RGBA32F:
            outFormat = GL_RGBA;
            outType = GL_FLOAT;
            break;

        // --- 特殊压缩格式 ---
        case GL_R11F_G11F_B10F:
            outFormat = GL_RGB;
            outType = GL_UNSIGNED_INT_10F_11F_11F_REV;
            break;

        default:
            outFormat = GL_RGBA;
            outType = GL_UNSIGNED_BYTE;
            break;
        }
    }

    //  根据深度内部格式推导 glTexImage2D 所需的 format/type
    void GetDepthFormatAndType(GLenum internalFormat, GLenum &outFormat, GLenum &outType)
    {
        switch (internalFormat)
        {
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
            outFormat = GL_DEPTH_COMPONENT;
            outType = GL_UNSIGNED_INT;
            break;
        case GL_DEPTH_COMPONENT32F:
            outFormat = GL_DEPTH_COMPONENT;
            outType = GL_FLOAT;
            break;
        case GL_DEPTH24_STENCIL8:
            outFormat = GL_DEPTH_STENCIL;
            outType = GL_UNSIGNED_INT_24_8;
            break;
        case GL_DEPTH32F_STENCIL8:
            outFormat = GL_DEPTH_STENCIL;
            outType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            break;
        default:
            outFormat = GL_DEPTH_COMPONENT;
            outType = GL_FLOAT;
            break;
        }
    }

    //  深度内部格式是否为深度+模板组合
    bool IsDepthStencil(GLenum internalFormat)
    {
        return internalFormat == GL_DEPTH24_STENCIL8 || internalFormat == GL_DEPTH32F_STENCIL8;
    }

    // ==================== FBO生命周期 ====================

    FBO::FBO(const FBOSpec &spec)
        : m_spec(spec)
    {
        Create();
    }

    FBO::~FBO()
    {
        Destroy();
    }

    //  判断FBO是否可用
    bool FBO::IsValid() const
    {
        return m_isValid;
    }

    // ==================== FBO绑定 ====================

    void FBO::FBOBind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    }

    void FBO::FBOUnbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // ==================== FBO配置比对 ====================

    //  忽略宽高，只比较配置(宽高单独判断)
    bool FBO::MatchConfig(const FBOSpec &spec) const
    {
        return SameConfig(m_spec, spec);
    }

    //  判断宽高是否一致
    bool FBO::MatchSize(int width, int height) const
    {
        return m_spec.width == width && m_spec.height == height;
    }

    //  判断spec是否一致
    bool FBO::MatchSpec(const FBOSpec &spec, int width, int height) const
    {
        return MatchSize(width, height) && MatchConfig(spec);
    }

    // ==================== 尺寸操作 ====================

    //  尺寸变化时重建
    bool FBO::Resize(int width, int height)
    {
        //  防御: 防止 0/负尺寸
        if (width <= 0 || height <= 0)
        {
            std::cerr << "FBO::Resize: invalid size " << width << "x" << height << std::endl;
            return false;
        }

        if (MatchSize(width, height))
        {
            return true; //  尺寸未变，无需重建
        }

        // ---------- 保存现场（旧对象可能正被绑定） ----------
        GLint prevRead = 0, prevDraw = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
        const GLuint oldFBO = m_FBO;
        // ---------- 保存现场（旧对象可能正被绑定） ----------

        //  更新尺寸并重建（配置不变）
        m_spec.width = width;
        m_spec.height = height;

        Destroy(); //  旧对象销毁（若被绑定，GL 会把绑定点重置为 0）
        Create();  //  内部会恢复成"重建前"的现场

        // ---------- 恢复现场（旧名字已失效，不能绑回去；原来绑着本对象的改绑新对象） ----------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, (static_cast<GLuint>(prevRead) == oldFBO) ? m_FBO : static_cast<GLuint>(prevRead));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (static_cast<GLuint>(prevDraw) == oldFBO) ? m_FBO : static_cast<GLuint>(prevDraw));
        // ---------- 恢复现场（旧名字已失效，不能绑回去；原来绑着本对象的改绑新对象） ----------

        return m_isValid;
    }

    //  配置时原地重建
    bool FBO::Rebuild(const FBOSpec &spec, int width, int height)
    {
        //  防御: 防止 0/负尺寸
        if (width <= 0 || height <= 0)
        {
            std::cerr << "FBO::Rebuild: invalid size " << width << "x" << height << std::endl;
            return false;
        }

        bool configChanged = !MatchConfig(spec);
        bool sizeChanged = !MatchSize(width, height);

        if (!configChanged && !sizeChanged)
        {
            return true;
        }

        // ---------- 保存现场 ----------
        GLint prevRead = 0, prevDraw = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
        const GLuint oldFBO = m_FBO;
        // ---------- 保存现场 ----------

        if (configChanged)
        {
            m_spec = spec;
        }

        m_spec.width = width;
        m_spec.height = height;

        Destroy();
        Create();

        // ---------- 恢复现场 ----------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, (static_cast<GLuint>(prevRead) == oldFBO) ? m_FBO : static_cast<GLuint>(prevRead));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (static_cast<GLuint>(prevDraw) == oldFBO) ? m_FBO : static_cast<GLuint>(prevDraw));
        // ---------- 恢复现场 ----------

        return m_isValid;
    }

    int FBO::GetWidth() const
    {
        return m_spec.width;
    }

    int FBO::GetHeight() const
    {
        return m_spec.height;
    }

    // ==================== 渲染目标绑定 ====================

    //  仅作为 blit / resolve 的 READ 源(不设视口)
    void FBO::ReadBind()
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
    }

    //  仅作为 blit / resolve 的 DRAW 目标(不设视口)
    void FBO::DrawBind()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
    }

    //  glDrawBuffer(GL_NONE)(深度-only 复用)
    void FBO::SetNoColorWrites()
    {
        if (!m_isValid)
        {
            return;
        }

        GLint prevRead = 0, prevDraw = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
        glDrawBuffer(GL_NONE); // 改的是 this 的状态（持久）
        glReadBuffer(GL_NONE);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(prevRead));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    }

    // ==================== 清屏 ====================

    void FBO::ClearColor(int index, const glm::vec4 &clearColor)
    {
        if (!m_isValid)
        {
            return;
        }
        if (index < 0 || index >= (int)m_colorTextures.size())
        {
            return;
        }

        // ---------- 绑定本对象（glClearBuffer* 作用于当前 DRAW 帧缓冲） ----------
        GLint prevDraw = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
        // ---------- 绑定本对象（glClearBuffer* 作用于当前 DRAW 帧缓冲） ----------

        glClearBufferfv(GL_COLOR, index, glm::value_ptr(clearColor));

        // ---------- 恢复现场 ----------
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    }

    void FBO::ClearColor(const glm::vec4 &clearColor)
    {
        if (!m_isValid)
        {
            return;
        }

        // ---------- 绑定本对象（glClearBuffer* 作用于当前 DRAW 帧缓冲） ----------
        GLint prevDraw = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
        // ---------- 绑定本对象（glClearBuffer* 作用于当前 DRAW 帧缓冲） ----------

        for (size_t i = 0; i < m_colorTextures.size(); ++i)
        {
            glClearBufferfv(GL_COLOR, (GLint)i, glm::value_ptr(clearColor));
        }

        // ---------- 恢复现场 ----------
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    }

    void FBO::ClearDepth(float depth)
    {
        if (!m_isValid || !m_spec.depthAttachment.enabled)
        {
            return;
        }

        // ---------- 绑定本对象（glClearBuffer* 作用于当前 DRAW 帧缓冲） ----------
        GLint prevDraw = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
        // ---------- 绑定本对象（glClearBuffer* 作用于当前 DRAW 帧缓冲） ----------

        glClearBufferfv(GL_DEPTH, 0, &depth);

        // ---------- 恢复现场 ----------
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    }

    void FBO::ClearStencil(int stencil)
    {
        if (!m_isValid || !m_spec.depthAttachment.enabled)
        {
            return;
        }
        if (!IsDepthStencil(m_spec.depthAttachment.internalFormat))
        {
            return;
        }

        // ---------- 绑定本对象（glClearBuffer* 作用于当前 DRAW 帧缓冲） ----------
        GLint prevDraw = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
        // ---------- 绑定本对象（glClearBuffer* 作用于当前 DRAW 帧缓冲） ----------

        glClearBufferiv(GL_STENCIL, 0, &stencil);

        // ---------- 恢复现场 ----------
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    }

    void FBO::ClearAll()
    {
        if (!m_isValid)
        {
            return;
        }

        // ---------- 绑定本对象（glClearBuffer* 作用于当前 DRAW 帧缓冲） ----------
        GLint prevDraw = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
        // ---------- 绑定本对象（glClearBuffer* 作用于当前 DRAW 帧缓冲） ----------

        // 清所有颜色附件
        const glm::vec4 black(0.0f, 0.0f, 0.0f, 1.0f);
        for (size_t i = 0; i < m_colorTextures.size(); ++i)
        {
            glClearBufferfv(GL_COLOR, (GLint)i, glm::value_ptr(black));
        }

        // 清深度/模板
        if (m_spec.depthAttachment.enabled)
        {
            if (IsDepthStencil(m_spec.depthAttachment.internalFormat))
            {
                glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
            }
            else
            {
                const GLfloat one = 1.0f;
                glClearBufferfv(GL_DEPTH, 0, &one);
            }
        }

        // ---------- 恢复现场 ----------
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    }

    // ==================== MSAA解析 ====================

    //  是否多重采样
    bool FBO::IsMultiSampled() const
    {
        return m_spec.samples > 1;
    }

    //  MSAA->单采样
    bool FBO::ResolveTo(FBO *target)
    {
        if (!target || target == this || !m_isValid || !target->m_isValid)
        {
            std::cerr << "ResolveTo: Invalid FBO specified" << std::endl;
            return false;
        }
        //  两个都是默认 FBO，没有意义
        if (m_FBO == 0 && target->m_FBO == 0)
        {
            std::cerr << "ResolveTo: Both source and target are default framebuffer" << std::endl;
            return false;
        }

        //  前置检查：源必须是 MSAA，目标必须不是 MSAA
        if (!IsMultiSampled())
        {
            // 不是 MSAA，没有 resolve 的意义
            std::cerr << "ResolveTo: source must be multisampled" << std::endl;
            return false;
        }
        if (target->IsMultiSampled())
        {
            // MSAA -> MSAA 不是 resolve，用 BlitTo 更合适
            std::cout << "ResolveTo: target must be non-multisampled" << std::endl;
            return false;
        }

        //  尺寸必须一致（规范要求，否则 GL_INVALID_OPERATION）
        if (GetWidth() != target->GetWidth() || GetHeight() != target->GetHeight())
        {
            std::cerr << "ResolveTo: target must have the same dimensions as source" << std::endl;
            return false;
        }

        //  两边都要有颜色附件，否则 blit 是静默 no-op
        if (!HasColorAttachment() || !target->HasColorAttachment())
        {
            std::cerr << "ResolveTo: both source and target need a color attachment" << std::endl;
            return false;
        }

        //  直接复用 BlitTo，参数固定
        //  mask 只取颜色；filter 必须是 GL_NEAREST
        BlitTo(target, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    //  MSAA->屏幕（默认帧缓冲）
    bool FBO::ResolveToScreen(int dstWidth, int dstHeight)
    {
        //  FBO可用性判断
        if (!m_isValid || m_FBO == 0)
        {
            std::cerr << "ResolveToScreen: invalid source FBO" << std::endl;
            return false;
        }

        //  非 MSAA 不需要 resolve（可直接改用 BlitToDefaultFramebuffer 上屏）
        if (!IsMultiSampled())
        {
            std::cerr << "ResolveToScreen: source is not multisampled (no resolve needed)" << std::endl;
            return false;
        }

        //  MSAA 解析不允许缩放：源尺寸必须等于屏幕尺寸
        //  （这里同时充当"窗口尺寸与 FBO 是否同步"的断言）
        if (GetWidth() != dstWidth || GetHeight() != dstHeight)
        {
            std::cerr << "ResolveToScreen: target must have the same dimensions as source" << std::endl;
            return false;
        }

        if (!HasColorAttachment())
        {
            std::cerr << "ResolveToScreen: source has no color attachment" << std::endl;
            return false;
        }

        //  只解析 0 号颜色附件；深度/模板不上屏，也不存在"深度解析"
        BlitToDefaultFramebuffer(dstWidth, dstHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    // ==================== Blit ====================

    //  Blit：拷贝FBO内容到另一个FBO
    bool FBO::BlitTo(FBO *target, GLbitfield mask, GLenum filter)
    {
        //  前置检查：源和目标都必须有效，且不能是同一个 FBO
        if (!target || target == this || !m_isValid || !target->m_isValid)
        {
            std::cerr << "BlitTo: Invalid FBO specified" << std::endl;
            return false;
        }

        const GLbitfield validMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        if ((mask & ~validMask) != 0 || mask == 0)
        {
            std::cerr << "BlitTo: invalid mask" << std::endl;
            return false;
        }
        if (filter != GL_NEAREST && filter != GL_LINEAR)
        {
            std::cerr << "BlitTo: invalid filter (must be GL_NEAREST or GL_LINEAR)" << std::endl;
            return false;
        }

        const bool srcMS = IsMultiSampled();
        const bool dstMS = target->IsMultiSampled();
        const bool sameSize = (GetWidth() == target->GetWidth() && GetHeight() == target->GetHeight());
        const bool hasDS = (mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) != 0;

        //  涉及多重采样时不允许缩放（解析/上采样都要求矩形一致）
        if ((srcMS || dstMS) && !sameSize)
        {
            std::cerr << "BlitTo: multisampled blit requires identical dimensions" << std::endl;
            return false;
        }
        //  MSAA -> MSAA 是逐采样拷贝，样本数必须一致
        if (srcMS && dstMS && m_spec.samples != target->m_spec.samples)
        {
            std::cerr << "BlitTo: MSAA->MSAA requires identical sample counts" << std::endl;
            return false;
        }

        //  深度/模板没有"解析"：采样数必须一致
        //  （格式兼容性此处未校验，目前只保证"有附件"）
        if (hasDS && m_spec.samples != target->m_spec.samples)
        {
            std::cerr << "BlitTo: depth/stencil blit requires identical sample counts" << std::endl;
            return false;
        }
        //  附件存在性：缺失时 blit 是静默 no-op，这里明确拦掉
        if ((mask & GL_COLOR_BUFFER_BIT) && (!HasColorAttachment() || !target->HasColorAttachment()))
        {
            std::cerr << "BlitTo: color blit requires color attachments on both sides" << std::endl;
            return false;
        }
        if (mask & GL_DEPTH_BUFFER_BIT)
        {
            if (!HasDepthAttachment() || !target->HasDepthAttachment())
            {
                std::cerr << "BlitTo: depth blit requires depth attachments on both sides" << std::endl;
                return false;
            }
        }
        if (mask & GL_STENCIL_BUFFER_BIT)
        {
            if (!HasStencilAttachment() || !target->HasStencilAttachment())
            {
                std::cerr << "BlitTo: stencil blit requires stencil attachments on both sides" << std::endl;
                return false;
            }
        }

        // ---------- 保存状态 ----------
        GLint prevRead = 0, prevDraw = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
        //  保存并关闭 scissor，避免 blit 被裁掉
        GLboolean scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
        if (scissorEnabled)
        {
            glDisable(GL_SCISSOR_TEST);
        }

        // ---------- 执行 ----------
        //  绑定源为 READ，目标为 DRAW
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->m_FBO);

        GLenum effectiveFilter = filter;
        if (srcMS || dstMS || hasDS)
        {
            effectiveFilter = GL_NEAREST; //  规范强制
        }

        //  计算源/目标矩形（左下原点，覆盖整张纹理）
        GLint srcW = static_cast<GLint>(GetWidth());
        GLint srcH = static_cast<GLint>(GetHeight());
        GLint dstW = static_cast<GLint>(target->GetWidth());
        GLint dstH = static_cast<GLint>(target->GetHeight());

        //  执行 blit
        glBlitFramebuffer(
            0, 0, srcW, srcH,
            0, 0, dstW, dstH,
            mask,
            effectiveFilter);

        // ---------- 恢复状态 ----------
        if (scissorEnabled)
        {
            glEnable(GL_SCISSOR_TEST);
        }
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(prevRead));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    }

    //  Blit：拷贝FBO内容到默认帧缓冲(屏幕)
    bool FBO::BlitToDefaultFramebuffer(int dstWidth, int dstHeight, GLbitfield mask, GLenum filter)
    {
        if (!m_isValid || m_FBO == 0)
        {
            std::cerr << "BlitToDefaultFramebuffer: invalid source FBO" << std::endl;
            return false;
        }
        if (dstWidth <= 0 || dstHeight <= 0)
        {
            std::cerr << "BlitToDefaultFramebuffer: invalid target size" << std::endl;
            return false;
        }

        const GLbitfield validMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        if ((mask & ~validMask) != 0 || mask == 0)
        {
            std::cerr << "BlitToDefaultFramebuffer: invalid mask" << std::endl;
            return false;
        }
        if (filter != GL_NEAREST && filter != GL_LINEAR)
        {
            std::cerr << "BlitToDefaultFramebuffer: invalid filter" << std::endl;
            return false;
        }

        //  深度/模板上屏无意义，且默认帧缓冲不保证有 → 剥离
        if (mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
        {
            std::cerr << "BlitToDefaultFramebuffer: depth/stencil ignored when blitting to screen" << std::endl;
            mask &= ~(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }
        if (!HasColorAttachment())
        {
            std::cerr << "BlitToDefaultFramebuffer: source has no color attachment" << std::endl;
            return false;
        }

        const bool srcMS = IsMultiSampled();

        // ---------- 保存状态 ----------
        GLint prevRead = 0, prevDraw = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
        const GLboolean scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
        if (scissorEnabled)
        {
            glDisable(GL_SCISSOR_TEST);
        }

        // ---------- 查询窗口采样数（GL_SAMPLES 取当前 DRAW 帧缓冲） ----------
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        GLint dstSamples = 0;
        glGetIntegerv(GL_SAMPLES, &dstSamples);
        const bool dstMS = (dstSamples > 1);

        auto restoreState = [&]()
        {
            if (scissorEnabled)
            {
                glEnable(GL_SCISSOR_TEST);
            }
            glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(prevRead));
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
        };

        //  涉及多重采样时不允许缩放
        if ((srcMS || dstMS) && (GetWidth() != dstWidth || GetHeight() != dstHeight))
        {
            std::cerr << "BlitToDefaultFramebuffer: multisampled blit requires identical dimensions" << std::endl;
            restoreState();
            return false;
        }
        //  双 MSAA 是逐采样拷贝（不是解析），样本数必须一致
        if (srcMS && dstMS && m_spec.samples != dstSamples)
        {
            std::cerr << "BlitToDefaultFramebuffer: MSAA->MSAA requires identical sample counts" << std::endl;
            restoreState();
            return false;
        }

        // ---------- 执行 ----------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);

        GLenum effectiveFilter = (srcMS || dstMS) ? GL_NEAREST : filter;

        glBlitFramebuffer(0, 0, GetWidth(), GetHeight(),
                          0, 0, dstWidth, dstHeight,
                          mask, effectiveFilter);

        restoreState();
    }

    // ==================== 附件查询 ====================

    bool FBO::HasColorAttachment() const
    {
        return !m_colorTextures.empty();
    }
    bool FBO::HasDepthAttachment() const
    {
        return m_depthTexture != nullptr;
    }
    bool FBO::HasStencilAttachment() const
    {
        if (!m_depthTexture)
        {
            return false;
        }
        GLenum internalFormat = m_spec.depthAttachment.internalFormat;
        return IsDepthStencil(internalFormat);
    }

    // ==================== 获取附件 ====================

    std::shared_ptr<Texture> FBO::GetColorAttachment(int index) const
    {
        if (index < 0 || static_cast<size_t>(index) >= m_colorTextures.size())
        {
            return nullptr;
        }
        return m_colorTextures[index];
    }

    //  按语义名(MRT/延迟)
    std::shared_ptr<Texture> FBO::GetColorAttachment(const std::string &name) const
    {
        //  空名不参与匹配(无名附件用下标接口)
        if (name.empty())
        {
            return nullptr;
        }

        //  规格与资源同序: 在规格中查名, 用查到的下标取真实纹理
        for (size_t i = 0; i < m_spec.colorAttachments.size(); ++i)
        {
            if (m_spec.colorAttachments[i].name == name)
            {
                return m_colorTextures[i];
            }
        }

        return nullptr; //  未找到
    }

    std::shared_ptr<Texture> FBO::GetDepthAttachment() const
    {
        return m_depthTexture;
    }

    // ==================== FBO信息查询 ====================

    GLuint FBO::GetFboID() const
    {
        return m_FBO;
    }

    const FBOSpec &FBO::GetSpec() const
    {
        return m_spec;
    }

    size_t FBO::GetColorAttachmentCount() const
    {
        return m_colorTextures.size();
    }

    // ==================== FBO类私有函数 ====================

    void FBO::Create()
    {
        m_isValid = false; //  初始标记无效，创建失败时不误用

        //  无附件尽早退出
        if (m_spec.colorAttachments.empty() && !m_spec.depthAttachment.enabled)
        {
            std::cerr << "FBO::Create: no attachments!" << std::endl;
            return;
        }

        //  fbo创建失败则返回
        glGenFramebuffers(1, &m_FBO);
        if (m_FBO == 0)
        {
            std::cerr << "FBO::Create: glGenFramebuffers failed!" << std::endl;
            return;
        }

        // ---------- 保存现场（此后所有失败都要走到末尾的恢复） ----------
        GLint prevRead = 0, prevDraw = 0;
        GLint prevTex2D = 0, prevTex2DMS = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTex2D);
        glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE, &prevTex2DMS);

        bool SaveTag = true; //  统一出口标志
        // ---------- 保存现场（此后所有失败都要走到末尾的恢复） ----------

        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

        // 1. 颜色附件(支持 MRT)
        m_colorTextures.clear();
        for (size_t i = 0; i < m_spec.colorAttachments.size(); ++i)
        {
            const auto &att = m_spec.colorAttachments[i];
            auto tex = Texture::CreateEmpty();
            if (!tex)
            {
                std::cerr << "FBO::Create: texture creation failed!" << std::endl;
                SaveTag = false;
                break;
            }

            GLenum target = (m_spec.samples > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
            glBindTexture(target, tex->GetID());

            if (m_spec.samples > 1)
            {
                glTexImage2DMultisample(target, m_spec.samples, att.internalFormat, m_spec.width, m_spec.height, GL_TRUE);
            }
            else
            {
                GLenum format, type;
                GetColorFormatAndType(att.internalFormat, format, type);
                glTexImage2D(target, 0, att.internalFormat, m_spec.width, m_spec.height, 0, format, type, nullptr);

                glTexParameteri(target, GL_TEXTURE_MIN_FILTER, att.sampler.minFilter);
                glTexParameteri(target, GL_TEXTURE_MAG_FILTER, att.sampler.magFilter);
                glTexParameteri(target, GL_TEXTURE_WRAP_S, att.sampler.wrapS);
                glTexParameteri(target, GL_TEXTURE_WRAP_T, att.sampler.wrapT);
            }

            //  挂载到 FBO 的第 i 号颜色槽
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i), target, tex->GetID(), 0);
            m_colorTextures.push_back(tex);
        }

        // 2. 深度/模板附件
        m_depthTexture.reset();
        if (SaveTag && m_spec.depthAttachment.enabled)
        {
            const auto &d = m_spec.depthAttachment;
            auto depthTex = Texture::CreateEmpty();
            if (!depthTex)
            {
                std::cerr << "FBO::Create: depth texture creation failed!" << std::endl;
                SaveTag = false;
            }
            else
            {
                GLenum target = (m_spec.samples > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
                glBindTexture(target, depthTex->GetID());

                if (m_spec.samples > 1)
                {
                    glTexImage2DMultisample(target, m_spec.samples, d.internalFormat, m_spec.width, m_spec.height, GL_TRUE);
                }
                else
                {
                    GLenum format, type;
                    GetDepthFormatAndType(d.internalFormat, format, type);
                    glTexImage2D(target, 0, d.internalFormat, m_spec.width, m_spec.height, 0, format, type, nullptr);

                    //  深度纹理必须使用 NEAREST 采样
                    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, d.sampler.minFilter);
                    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, d.sampler.magFilter);
                    glTexParameteri(target, GL_TEXTURE_WRAP_S, d.sampler.wrapS);
                    glTexParameteri(target, GL_TEXTURE_WRAP_T, d.sampler.wrapT);
                }

                GLenum attachment = IsDepthStencil(d.internalFormat) ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
                glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, depthTex->GetID(), 0);
                m_depthTexture = depthTex;
            }
        }

        // 3. 指定绘制哪些颜色缓冲区
        if (SaveTag)
        {
            if (!m_spec.colorAttachments.empty())
            {
                std::vector<GLenum> drawBuffers(m_spec.colorAttachments.size());
                for (size_t i = 0; i < m_spec.colorAttachments.size(); ++i)
                {
                    drawBuffers[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
                }
                glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
            }
            else
            {
                //  深度-only FBO 必须显式关闭颜色绘制，否则不完整
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);
            }
        }

        // 4. 检查完整性
        if (SaveTag && glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "FBO Error: Incomplete! width=" << m_spec.width << " height=" << m_spec.height << std::endl;

            //  创建失败 → 清理资源
            SaveTag = false;
        }

        if (SaveTag)
        {
            m_isValid = true;
        }
        else
        {
            Destroy();
        }

        /// ---------- 恢复现场（唯一出口） ----------
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(prevTex2D));
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(prevTex2DMS));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(prevRead));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    }

    void FBO::Destroy()
    {
        if (m_FBO)
        {
            glDeleteFramebuffers(1, &m_FBO);
            m_FBO = 0;
        }
        m_colorTextures.clear();
        m_depthTexture.reset();
        m_isValid = false;
    }
}