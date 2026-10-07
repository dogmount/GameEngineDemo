#ifndef GL_ENGINE_FRAME_BUFFER_MANAGER_H
#define GL_ENGINE_FRAME_BUFFER_MANAGER_H
#pragma once

#include "shaderAPI/FBO.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace GL_engine
{
    class Texture;

    //  帧缓冲管理器：管理一组按名称索引的 FBO 的生命周期(创建/复用/重建/销毁/查询)
    //  渲染管线(RenderPipeline)通过本类为每个 Pass 构建其输出 FBO
    class FBOManager
    {
    public:
        FBOManager() = default;
        ~FBOManager() = default;

        FBOManager(const FBOManager &) = delete;
        FBOManager &operator=(const FBOManager &) = delete;
        FBOManager(FBOManager &&) = delete;            //  移动构造
        FBOManager &operator=(FBOManager &&) = delete; //  移动赋值

        // ---- 生命周期 ----
        FBO *Create(const std::string &name, const FBOSpec &spec, int windowWidth = 1920, int windowHeight = 1080); //  windowWidth/windowHeight 用于解析 windowRelative 的 FBO 尺寸

        //  只记录窗口尺寸，不触发重建(用于初始化时提前同步)
        void SetWindowSize(int width, int height);

        void Remove(const std::string &name);         //  销毁指定 FBO
        void Clear();                                 //  销毁全部 FBO
        bool Contains(const std::string &name) const; //  是否存在指定名称的 FBO
        size_t GetCount() const;                      //  获取 FBO 数量
        std::vector<std::string> GetNames() const;    //  获取所有 FBO 名称列表

        // ---- 查找 / 窗口 ----
        FBO *GetFBO(const std::string &name) const; //  获取已存在的 FBO(不存在返回 nullptr)
        void Resize(int width, int height);         //  窗口尺寸变化：重建所有 windowRelative 的 FBO
        int GetWindowWidth() const;
        int GetWindowHeight() const;

        // ---- 附件转发 ----
        std::shared_ptr<Texture> GetColorAttachment(const std::string &name, int index = 0) const;                     //  按名称获取颜色附件纹理
        std::shared_ptr<Texture> GetColorAttachment(const std::string &name, const std::string &attachmentName) const; //  MRT/延迟着色: 按 FBO 名 + 附件名取
        std::shared_ptr<Texture> GetDepthAttachment(const std::string &name) const;                                    //  按名称获取深度附件纹理

        //  ---- Blit（通用路径）----
        bool Blit(const std::string &srcName, const std::string &dstName, GLbitfield mask = GL_COLOR_BUFFER_BIT, GLenum filter = GL_LINEAR);          //  通用路径：FBO -> FBO
        bool BlitToScreen(const std::string &srcName, int dstWidth, int dstHeight, GLbitfield mask = GL_COLOR_BUFFER_BIT, GLenum filter = GL_LINEAR); //  通用上屏：FBO -> 默认帧缓冲

        //  ---- Resolve（MSAA 闭环，自动分流）----
        bool Resolve(const std::string &srcName, const std::string &dstName, GLbitfield mask = GL_COLOR_BUFFER_BIT, GLenum filter = GL_LINEAR);          //  MSAA 闭环: FBO → FBO
        bool ResolveToScreen(const std::string &srcName, int dstWidth, int dstHeight, GLbitfield mask = GL_COLOR_BUFFER_BIT, GLenum filter = GL_LINEAR); //  最终上屏：FBO → 默认帧缓冲（按 MSAA 分流）
        bool ResolveToScreen(const std::string &srcName, GLbitfield mask = GL_COLOR_BUFFER_BIT, GLenum filter = GL_LINEAR);                              //  上屏：（使用管理器记录的窗口尺寸）

    private:
        //  把规格解析成实际像素尺寸
        //    Fixed          → spec.width/height
        //    WindowRelative → 窗口 × widthScale/heightScale
        void ResolveSize(const FBOSpec &spec, int &outWidth, int &outHeight) const;

        std::unordered_map<std::string, std::unique_ptr<FBO>> m_fbos; //  名称 -> FBO

        int m_windowWidth = 0;
        int m_windowHeight = 0;
    };
}

#endif