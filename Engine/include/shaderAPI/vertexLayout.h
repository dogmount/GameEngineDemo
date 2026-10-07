#ifndef VERTEX_LAYOUT_H
#define VERTEX_LAYOUT_H
#pragma once

#include <vector>

#include "glad/glad.h"

//  改进点记录——VertexElement添加归一化属性（暂定
namespace GL_engine
{
    struct VertexElement
    {
        GLuint index;    //  属性索引
        GLuint size;     //  组件数量
        GLuint type;     //  数据类型
        uint32_t offset; //  偏移量

        static constexpr int PositionIndex = 0; //  位置索引
        static constexpr int ColorIndex = 1;    //  颜色索引
        static constexpr int UVIndex = 2;       //  贴图索引
        static constexpr int NormalIndex = 3;   //  法线索引
        static constexpr int TangentIndex = 4;  //  切线索引
    };

    struct VertexLayout
    {
        std::vector<VertexElement> elements; //  顶点布局数组
        uint32_t stride = 0;                 //  顶点布局步长
    };
}

#endif