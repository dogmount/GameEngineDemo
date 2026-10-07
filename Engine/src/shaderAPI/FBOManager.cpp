#include "shaderAPI/FBOManager.h"
#include "shaderAPI/texture.h"

#include <iostream>
#include <algorithm>

namespace GL_engine
{
    //  按名称创建或获取 FBO
    FBO *FBOManager::Create(const std::string &name, const FBOSpec &spec, int windowWidth, int windowHeight)
    {
        if (name.empty())
        {
            std::cerr << "FBOManager::Create: empty name!" << std::endl;
            return nullptr;
        }

        //  先更新窗口尺寸(仅 >0 生效)，再解析 —— 顺序不能反
        SetWindowSize(windowWidth, windowHeight);

        int w = 1, h = 1;
        ResolveSize(spec, w, h);

        //  规格与尺寸均未变化时直接复用
        auto it = m_fbos.find(name);
        if (it != m_fbos.end())
        {
            FBO *fbo = it->second.get();

            if (fbo->MatchSpec(spec, w, h))
            {
                return fbo; //  规格与尺寸都没变，直接复用
            }

            if (!fbo->Rebuild(spec, w, h))
            {
                //  重建失败 → 移除，避免留下无效对象
                std::cerr << "FBOManager::Create: Rebuild failed for \"" << name << "\"!" << std::endl;
                m_fbos.erase(it);
                return nullptr;
            }

            return fbo; //  原地重建，指针地址不变
        }

        FBOSpec resolved = spec;
        resolved.width = w;
        resolved.height = h;

        auto fbo = std::make_unique<FBO>(resolved);
        FBO *ptr = fbo.get();

        if (!ptr->IsValid())
        {
            //  创建失败(样本数超限/附件组合不完整等) → 不入册, 避免拿到坏资源
            std::cerr << "FBOManager::Create: FBO \"" << name << "\" is invalid, not registered!" << std::endl;
            return nullptr;
        }

        m_fbos.emplace(name, std::move(fbo));
        return ptr;
    }

    //  只记录窗口尺寸，不触发重建(用于初始化时提前同步)
    void FBOManager::SetWindowSize(int width, int height)
    {
        if (width > 0 && height > 0)
        {
            m_windowWidth = width;
            m_windowHeight = height;
        }
    }

    //  销毁指定 FBO
    void FBOManager::Remove(const std::string &name)
    {
        m_fbos.erase(name);
    }

    //  销毁全部
    void FBOManager::Clear()
    {
        m_fbos.clear();
    }

    //  是否存在
    bool FBOManager::Contains(const std::string &name) const
    {
        return m_fbos.find(name) != m_fbos.end();
    }

    //  获取 FBO 数量
    size_t FBOManager::GetCount() const
    {
        return m_fbos.size();
    }

    //  获取所有 FBO 名称列表
    std::vector<std::string> FBOManager::GetNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_fbos.size());

        for (const auto &entry : m_fbos)
        {
            names.push_back(entry.first); //  注意: unordered_map 顺序不保证
        }
        return names;
    }

    //  调试/解析参考
    int FBOManager::GetWindowWidth() const
    {
        return m_windowWidth;
    }
    int FBOManager::GetWindowHeight() const
    {
        return m_windowHeight;
    }

    //  获取已存在 FBO
    FBO *FBOManager::GetFBO(const std::string &name) const
    {
        auto it = m_fbos.find(name);
        if (it == m_fbos.end())
        {
            return nullptr;
        }

        FBO *fbo = it->second.get();
        return fbo->IsValid() ? fbo : nullptr; //  防御：失效对象不外传
    }

    //  窗口尺寸变化：重建所有 windowRelative 的 FBO
    void FBOManager::Resize(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            std::cerr << "FBOManager::Resize: invalid size " << width << "x" << height << std::endl;
            return;
        }

        m_windowWidth = width;
        m_windowHeight = height;

        std::vector<std::string> dead; //  重建失败的先记下来，循环后再删(避免迭代器失效)

        for (auto &entry : m_fbos)
        {
            FBO *fbo = entry.second.get();

            const FBOSpec &spec = fbo->GetSpec(); //  解析后规格(保留了 windowRelative/scale)
            if (spec.source != FBOSizeSource::WindowRelative)
            {
                continue;
            }

            int w = 1, h = 1;
            ResolveSize(spec, w, h);

            if (!fbo->Resize(w, h))
            {
                std::cerr << "FBOManager::Resize: rebuild failed for \"" << entry.first << "\"" << std::endl;
                dead.push_back(entry.first);
            }
        }

        //  清除重建失败的 FBO
        for (const std::string &name : dead)
        {
            m_fbos.erase(name);
        }
    }

    //  按名称获取颜色附件纹理
    std::shared_ptr<Texture> FBOManager::GetColorAttachment(const std::string &name, int index) const
    {
        FBO *fbo = GetFBO(name);
        return fbo ? fbo->GetColorAttachment(index) : nullptr;
    }

    //  MRT/延迟着色: 按 FBO 名 + 附件名取
    std::shared_ptr<Texture> FBOManager::GetColorAttachment(const std::string &name, const std::string &attachmentName) const
    {
        FBO *fbo = GetFBO(name);
        return fbo ? fbo->GetColorAttachment(attachmentName) : nullptr;
    }

    //  按名称获取深度附件纹理
    std::shared_ptr<Texture> FBOManager::GetDepthAttachment(const std::string &name) const
    {
        FBO *fbo = GetFBO(name);
        return fbo ? fbo->GetDepthAttachment() : nullptr;
    }

    //  FBO -> FBO
    bool FBOManager::Blit(const std::string &srcName, const std::string &dstName, GLbitfield mask, GLenum filter)
    {
        FBO *src = GetFBO(srcName);
        FBO *dst = GetFBO(dstName);

        if (!src || !dst)
        {
            std::cerr << "FBOManager::Blit: FBO not found! src=\"" << srcName << "\" dst=\"" << dstName << "\"" << std::endl;
            return false;
        }

        if (src == dst)
        {
            std::cerr << "FBOManager::Blit: src and dst are the same FBO!" << std::endl;
            return false;
        }

        return src->BlitTo(dst, mask, filter);
    }

    //  FBO -> 默认帧缓冲
    bool FBOManager::BlitToScreen(const std::string &srcName, int dstWidth, int dstHeight, GLbitfield mask, GLenum filter)
    {
        FBO *src = GetFBO(srcName);
        if (!src)
        {
            std::cerr << "FBOManager::BlitToScreen: FBO not found! src=\"" << srcName << "\"" << std::endl;
            return false;
        }

        //  深度/模板会被 FBO 内部剥离；MSAA 源要求与目标同尺寸
        return src->BlitToDefaultFramebuffer(dstWidth, dstHeight, mask, filter);
    }

    //  MSAA 闭环: FBO → FBO
    bool FBOManager::Resolve(const std::string &srcName, const std::string &dstName, GLbitfield mask, GLenum filter)
    {
        FBO *src = GetFBO(srcName);
        FBO *dst = GetFBO(dstName);

        if (!src || !dst)
        {
            std::cerr << "FBOManager::Resolve: FBO not found! src=\"" << srcName << "\" dst=\"" << dstName << "\"" << std::endl;
            return false;
        }

        if (src == dst)
        {
            std::cerr << "FBOManager::Resolve: src and dst are the same FBO!" << std::endl;
            return false;
        }

        //  "真正的 resolve" 的判定条件：
        //  只解析颜色，且 MSAA 源 → 单采样目标（filter 由规范强制 NEAREST，无需判断）
        const bool isMSAAResolve = (mask == GL_COLOR_BUFFER_BIT) && src->IsMultiSampled() && !dst->IsMultiSampled();

        if (isMSAAResolve)
        {
            //  窄语义入口：内部会再做一次方向/尺寸/颜色附件校验
            return src->ResolveTo(dst);
        }
        else
        {
            //  单采样缩放 / MSAA→MSAA 拷贝 / 深度模板搬运 / mask 含深度模板：交回通用路径，
            //  否则会被 FBO::ResolveTo 拒绝（它只接受"MSAA源→单采样目标 + 只颜色"）
            return src->BlitTo(dst, mask, filter);
        }
    }

    //  最终上屏: ResolveToScreen("LDR", w, h)
    bool FBOManager::ResolveToScreen(const std::string &srcName, int dstWidth, int dstHeight, GLbitfield mask, GLenum filter)
    {
        FBO *src = GetFBO(srcName);
        if (!src)
        {
            std::cerr << "FBOManager::ResolveToScreen: FBO not found! src=\"" << srcName << "\"" << std::endl;
            return false;
        }

        if (src->IsMultiSampled())
        {
            //  窄语义：只颜色 + GL_NEAREST，且要求源尺寸 == 目标尺寸（mask/filter 在此分支不生效）
            if (mask != GL_COLOR_BUFFER_BIT)
            {
                std::cerr << "FBOManager::ResolveToScreen: mask ignored for MSAA resolve!" << std::endl;
            }
            return src->ResolveToScreen(dstWidth, dstHeight);
        }
        else
        {
            //  单采样上屏：可缩放、可指定 mask/filter（复用 BlitToScreen）
            return BlitToScreen(srcName, dstWidth, dstHeight, mask, filter);
        }
    }

    //  上屏：（使用管理器记录的窗口尺寸）
    bool FBOManager::ResolveToScreen(const std::string &srcName, GLbitfield mask, GLenum filter)
    {
        if (m_windowWidth <= 0 || m_windowHeight <= 0)
        {
            std::cerr << "FBOManager::ResolveToScreen: window size unknown!" << std::endl;
            return false;
        }

        return ResolveToScreen(srcName, m_windowWidth, m_windowHeight, mask, filter);
    }

    //  根据规格与窗口尺寸解析 FBO 的实际尺寸
    void FBOManager::ResolveSize(const FBOSpec &spec, int &outWidth, int &outHeight) const
    {
        if (spec.source == FBOSizeSource::WindowRelative && m_windowWidth > 0 && m_windowHeight > 0)
        {
            outWidth = std::max(1, static_cast<int>(m_windowWidth * spec.widthScale));
            outHeight = std::max(1, static_cast<int>(m_windowHeight * spec.heightScale));
        }
        else
        {
            //  Fixed，或窗口尺寸未知(尚未 Resize/SetWindowSize) → 用 spec 的显式尺寸
            outWidth = std::max(1, spec.width);
            outHeight = std::max(1, spec.height);
        }
    }
}