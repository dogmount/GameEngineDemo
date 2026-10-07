#ifndef GL_ENGINE_FBO_H
#define GL_ENGINE_FBO_H
#pragma once

#include "glad/glad.h"
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <string>

namespace GL_engine
{
    class Texture;

    enum class FBOSizeSource
    {
        Fixed,
        WindowRelative
    };

    struct FBOAttachmentSampler
    {
        GLenum minFilter = GL_LINEAR;
        GLenum magFilter = GL_LINEAR;
        GLenum wrapS = GL_CLAMP_TO_EDGE;
        GLenum wrapT = GL_CLAMP_TO_EDGE;
    };

    //  颜色附件描述
    struct FBOColorAttachment
    {
        std::string name = "";
        bool useMipmap = false;             //  是否生成mipmap(仅 samples==1)
        GLenum internalFormat = GL_RGBA16F; //  内部格式(附件的数据类型)
        FBOAttachmentSampler sampler;       //  采样状态
    };

    //  深度/模板附件描述
    struct FBODepthAttachment
    {
        std::string name = "depth";
        bool enabled = true;                          //  是否创建深度附件
        GLenum internalFormat = GL_DEPTH_COMPONENT24; //  深度格式(数据类型)
        FBOAttachmentSampler sampler;                 //  采样状态
    };

    //  FBO 整体描述：根据该规格自定义构建一个帧缓冲
    struct FBOSpec
    {
        FBOSizeSource source = FBOSizeSource::Fixed; // 默认固定 1920x1080(向后兼容)

        //  帧缓冲宽高
        int width = 1920;
        int height = 1080;

        //  相对窗口宽度的缩放系数
        float widthScale = 1.0f;
        float heightScale = 1.0f;

        int samples = 1; //  多重采样，1 = 无MSAA

        std::vector<FBOColorAttachment> colorAttachments; //  颜色附件(支持 MRT 多渲染目标)
        FBODepthAttachment depthAttachment;               //  深度附件
    };

    inline bool operator==(const FBOAttachmentSampler &a, const FBOAttachmentSampler &b)
    {
        return a.minFilter == b.minFilter &&
               a.magFilter == b.magFilter &&
               a.wrapS == b.wrapS &&
               a.wrapT == b.wrapT;
    }

    inline bool operator==(const FBOColorAttachment &a, const FBOColorAttachment &b)
    {
        return a.useMipmap == b.useMipmap &&
               a.internalFormat == b.internalFormat &&
               a.sampler == b.sampler;
    }

    inline bool operator==(const FBODepthAttachment &a, const FBODepthAttachment &b)
    {
        return a.enabled == b.enabled &&
               a.internalFormat == b.internalFormat &&
               a.sampler == b.sampler;
    }

    //  配置比较
    inline bool SameConfig(const FBOSpec &a, const FBOSpec &b)
    {
        if (a.source != b.source)
        {
            return false;
        }
        if (a.source == FBOSizeSource::WindowRelative)
        {
            if (a.widthScale != b.widthScale || a.heightScale != b.heightScale)
            {
                return false;
            }
        }
        return a.samples == b.samples &&
               a.colorAttachments == b.colorAttachments &&
               a.depthAttachment == b.depthAttachment;
    }

    //  配置与实际生效尺寸比较
    inline bool operator==(const FBOSpec &a, const FBOSpec &b)
    {
        return SameConfig(a, b) &&
               a.width == b.width &&
               a.height == b.height;
    }

    //  单个帧缓冲对象：根据 FBOSpec 自定义构建颜色/深度附件
    //  生命周期(创建/重建/销毁)交由 FBOManager 管理器负责
    class FBO
    {
    public:
        FBO() = delete;
        FBO(const FBO &) = delete;
        FBO &operator=(const FBO &) = delete;

        explicit FBO(const FBOSpec &spec);
        ~FBO();

        //  FBO可用性判断
        bool IsValid() const;

        // ---- FBO绑定 ----
        void FBOBind();   //  绑定帧缓冲
        void FBOUnbind(); //  解绑回默认帧缓冲

        // ---- 配置比对 ----
        bool MatchConfig(const FBOSpec &spec) const;                      //  比对fbo配置
        bool MatchSize(int width, int height) const;                      //  比对fbo尺寸
        bool MatchSpec(const FBOSpec &spec, int width, int height) const; //  比对fbo配置+尺寸

        // ---- 尺寸 ----
        bool Resize(int width, int height);                       //  尺寸变化时重建
        bool Rebuild(const FBOSpec &spec, int width, int height); //  配置变化时重建
        int GetWidth() const;
        int GetHeight() const;

        // ---- 渲染目标绑定 ----
        void ReadBind();         //  作为READ源
        void DrawBind();         //  作为DRAW源
        void SetNoColorWrites(); //  禁止颜色写入(深度-only FBO)

        // ---- 清屏 ----

        void ClearColor(int index, const glm::vec4 &clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        void ClearColor(const glm::vec4 &clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        void ClearDepth(float depth = 1.0f);
        void ClearStencil(int stencil = 0);
        void ClearAll(); //  一次性清

        // ---- MSAA解析 ----
        bool IsMultiSampled() const;                       //  是否多重采样
        bool ResolveTo(FBO *target);                       //  MSAA->单采样
        bool ResolveToScreen(int dstWidth, int dstHeight); //  MSAA->屏幕（默认帧缓冲）

        // ---- Blit ----
        bool BlitTo(FBO *target, GLbitfield mask, GLenum filter = GL_LINEAR);                                   //  Blit：拷贝FBO内容到另一个FBO
        bool BlitToDefaultFramebuffer(int dstWidth, int dstHeight, GLbitfield mask, GLenum filter = GL_LINEAR); //  Blit：拷贝FBO内容到默认帧缓冲(屏幕)

        // ---- 附件查询 ----
        bool HasColorAttachment() const;
        bool HasDepthAttachment() const;
        bool HasStencilAttachment() const;

        // ---- 获取附件 ----
        std::shared_ptr<Texture> GetColorAttachment(int index = 0) const;
        std::shared_ptr<Texture> GetColorAttachment(const std::string &name) const; //  按语义名(MRT/延迟)
        std::shared_ptr<Texture> GetDepthAttachment() const;

        // ---- FBO信息查询 ----
        GLuint GetFboID() const;
        const FBOSpec &GetSpec() const;
        size_t GetColorAttachmentCount() const; //  颜色附件数量(MRT 遍历用)

    private:
        void Create();  //  创建帧缓冲
        void Destroy(); //  销毁帧缓冲

        GLuint m_FBO = 0; //  帧缓冲对象
        FBOSpec m_spec;   //  帧缓冲配置(含实际生效尺寸)

        bool m_isValid = false;

        std::vector<std::shared_ptr<Texture>> m_colorTextures; //  颜色纹理附件
        std::shared_ptr<Texture> m_depthTexture;               //  深度纹理附件
    };
}

#endif