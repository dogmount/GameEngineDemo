#include "IO_file/file_system.h"
#include "config.h"

#if defined _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#endif

#include <fstream>
#include <iostream>

namespace GL_engine
{
    std::filesystem::path FileSystem::GetExecutableFolder() const //  获取可执行文件夹
    {
#if defined _WIN32
        wchar_t buf[MAX_PATH];
        GetModuleFileNameW(NULL, buf, MAX_PATH);
        return std::filesystem::path(buf).remove_filename();
#elif defined(__APPLE__)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string tmp(size, '\0');
        _NSGetExecutablePath(tmp.data(), &size);
        return std::filesystem::weakly_canonical(std::filesystem::path(tmp).remove_filename());
#elif defined(__linux__)
        return std::filesystem::weakly_canonical(std::filesystem::read_symlink("/proc/self/exe").remove_filename());
#else
        return std::filesystem::current_path();
#endif
    }

    std::filesystem::path FileSystem::GetAssetsFolder() const //  获取资源文件夹
    {
#if defined(ASSETS_ROOT)
        auto path = std::filesystem::path(std::string(ASSETS_ROOT));
        if (std::filesystem::exists(path))
        {
            return path;
        }
#endif
        return std::filesystem::weakly_canonical(GetExecutableFolder() / "assets");
    }

    //  二进制加载文件
    std::vector<char> FileSystem::LoadFile(const std::filesystem::path &path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate); // std::ios::ate（at end）打开文件后，立即将文件指针移动到文件末尾。
        if (!file.is_open())
        {
            return {};
            std::cout << "Fail to Load File :: " << path.string() << std::endl;
        }

        auto size = file.tellg(); // 获取当前指针位置（即文件末尾的偏移量 = 文件大小）
        file.seekg(0);            // 将文件流的读取指针移动到文件的起始位置

        std::vector<char> buffer(size);

        if (!file.read(buffer.data(), size))
        {
            return {};
            std::cout << "Fail to Load File :: " << path.string() << std::endl;
        }

        return buffer;
    }

    //  相对路径加载资源文件
    std::vector<char> FileSystem::LoadAssetFile(const std::string &relativePath)
    {
        return LoadFile(GetAssetsFolder() / relativePath);
    }

    //  相对路径文件文本
    std::string FileSystem::LoadAssetFileText(const std::string &relativePath)
    {
        auto buffer = LoadAssetFile(relativePath);
        return std::string(buffer.begin(), buffer.end());
    }
}