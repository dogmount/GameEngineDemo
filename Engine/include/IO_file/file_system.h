#ifndef MY_FILE_SYSTEM_H
#define MY_FILE_SYSTEM_H
#pragma once

#include <filesystem>
#include <vector>
#include <string>

namespace GL_engine
{
    class FileSystem
    {
    public:
        std::filesystem::path GetExecutableFolder() const; //  获取可执行文件夹
        std::filesystem::path GetAssetsFolder() const;     //  获取资源文件夹

        std::vector<char> LoadFile(const std::filesystem::path &path);    //  二进制加载文件
        std::vector<char> LoadAssetFile(const std::string &relativePath); //  相对路径加载资源文件
        std::string LoadAssetFileText(const std::string &relativePath);   //  加载文件文本
    };
}

#endif