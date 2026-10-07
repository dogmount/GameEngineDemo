#ifndef MESH_H
#define MESH_H
#pragma once

#include "glad/glad.h"
#include "shaderAPI/vertexLayout.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <string>

namespace GL_engine
{
    //  圆周率
    static constexpr float PI = glm::pi<float>();

    class Mesh
    {
    public:
        //  带索引构造
        Mesh(const VertexLayout &layout, const std::vector<float> &vertices, const std::vector<uint32_t> &indices);

        //  无索引构造
        Mesh(const VertexLayout &layout, const std::vector<float> &vertices);

        ~Mesh();

        //  禁用拷贝构造/拷贝赋值
        Mesh(const Mesh &) = delete;
        Mesh &operator=(const Mesh &) = delete;

        void MeshBind();   //  绑定VAO
        void MeshUnbind(); //  解绑VAO
        void MeshDraw();   //  绘制图形

        //  部分绘制
        void DrawIndexedRange(uint32_t startIndex, uint32_t indexCount);

        //  更新动态
        void UpdateDynamic(const std::vector<float> &vertices);
        void UpdateDynamic(const std::vector<float> &vertices, const std::vector<uint32_t> &indices);

        //  全屏四边形
        static std::shared_ptr<Mesh> FullscreenQuad();

        //  2D平面
        static std::shared_ptr<Mesh> CreatePlane2D();

        //  3D创建平面
        static std::shared_ptr<Mesh> CreatePlane(float width = 1.0, float height = 1.0, int wSegment = 1, int hSegment = 1);

        //  3D创建箱子
        static std::shared_ptr<Mesh> CreateBox(float width = 1.0f, float height = 1.0f, float depth = 1.0f, float widthSegments = 1.0f, float heightSegments = 1.0f, float depthSegments = 1.0f);

        //  3D创建球体
        static std::shared_ptr<Mesh> CreateSphere(float radius = 1.0f, int widthSegments = 8, int heightSegments = 6, float phiStart = 0.0f, float phiLength = PI * 2.0f, float thetaStart = 0.0f, float thetaLength = PI);

        //  3D创建三棱柱：底面三角形（a,b,c)+挤出向量（棱方向与长度)
        static std::shared_ptr<Mesh> CreatePrism(const glm::vec3 &a = glm::vec3(0.0f, 0.0f, -0.5f), const glm::vec3 &b = glm::vec3(0.0f, 0.0f, 0.5f), const glm::vec3 &c = glm::vec3(0.0f, 1.0f, 0.5f), const glm::vec3 &extrude = glm::vec3(1.0f, 0.0f, 0.0f));

    private:
        static void AppendPlane(std::vector<float> &vertices, std::vector<uint32_t> &indices, int u, int v, int w, float udir, float vdir, float facePos, float width, float height, int gridX, int gridY);

        VertexLayout m_vertexLayout; //  顶点数据
        size_t m_vertexCount = 0;    //  顶点数据计数
        size_t m_indexCount = 0;     //  顶点索引计数

        GLuint m_VAO = 0;
        GLuint m_VBO = 0;
        GLuint m_EBO = 0;
    };
}

#endif