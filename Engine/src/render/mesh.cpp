#include "render/mesh.h"
#include "shaderAPI/shaderAPI.h"
#include "Engine/Engine.h"

namespace GL_engine
{
    //  带顶点索引的构造函数
    Mesh::Mesh(const VertexLayout &layout, const std::vector<float> &vertices, const std::vector<uint32_t> &indices)
    {
        m_vertexLayout = layout;

        auto &shaderAPI = Engine::GetInstance().GetShaderAPI();

        m_VBO = shaderAPI.CreateVertexBuffer(vertices); //  创建VBO
        m_EBO = shaderAPI.CreateIndexBuffer(indices);   //  创建EBO

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        //  遍历顶点布局
        for (auto &elment : m_vertexLayout.elements)
        {
            // 设置属性指针
            glVertexAttribPointer(elment.index, elment.size, elment.type, GL_FALSE, m_vertexLayout.stride, (void *)(uintptr_t)elment.offset); //  64位下uin32_t需类型转换为uintptr_t保证偏移量大小与指针大小对应

            // 启用顶点属性
            glEnableVertexAttribArray(elment.index);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

        //  解绑VAO
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        //  计算顶点数量
        m_vertexCount = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;

        //  计算索引数量
        m_indexCount = indices.size();
    }

    //  不带索引的构造函数
    Mesh::Mesh(const VertexLayout &layout, const std::vector<float> &vertices)
    {
        m_vertexLayout = layout;

        //  获取实例
        auto &shaderAPI = Engine::GetInstance().GetShaderAPI();

        //  创建顶点缓冲区
        m_VBO = shaderAPI.CreateVertexBuffer(vertices);

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        //  遍历顶点布局
        for (auto &elment : m_vertexLayout.elements)
        {
            // 设置属性指针
            glVertexAttribPointer(elment.index, elment.size, elment.type, GL_FALSE, m_vertexLayout.stride, (void *)(uintptr_t)elment.offset); //  64位下uin32_t需类型转换为uintptr_t保证偏移量大小与指针大小对应

            // 启用顶点属性
            glEnableVertexAttribArray(elment.index);
        }

        //  解绑VAO
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //  计算顶点数量
        m_vertexCount = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;
    }

    Mesh::~Mesh()
    {
        //  释放 GL 资源(须在上下文当前时执行; 场景销毁发生在 EngDestroy 的上下文存活期内, 没问题)
        if (m_VAO != 0)
        {
            glDeleteVertexArrays(1, &m_VAO);
            m_VAO = 0;
        }
        if (m_VBO != 0)
        {
            glDeleteBuffers(1, &m_VBO);
            m_VBO = 0;
        }
        if (m_EBO != 0)
        {
            glDeleteBuffers(1, &m_EBO);
            m_EBO = 0;
        }
    }

    void Mesh::MeshBind()
    {
        glBindVertexArray(m_VAO);
    }

    void Mesh::MeshUnbind()
    {
        glBindVertexArray(0);
    }

    //  绘制图形
    void Mesh::MeshDraw()
    {
        if (m_indexCount > 0) //  有顶点索引
        {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0);
        }
        else //  无顶点索引
        {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertexCount));
        }
    }

    //  部分绘制
    void Mesh::DrawIndexedRange(uint32_t startIndex, uint32_t indexCount)
    {
        if (indexCount == 0)
        {
            return;
        }
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount),
                       GL_UNSIGNED_INT, reinterpret_cast<void *>(static_cast<size_t>(startIndex) * sizeof(uint32_t)));
    }

    //  更新动态
    void Mesh::UpdateDynamic(const std::vector<float> &vertices)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_vertexCount = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;
    }
    void Mesh::UpdateDynamic(const std::vector<float> &vertices, const std::vector<uint32_t> &indices)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_vertexCount = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;

        if (m_EBO == 0)
        {
            m_EBO = Engine::GetInstance().GetShaderAPI().CreateIndexBuffer(indices);

            glBindVertexArray(m_VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
            glBindVertexArray(0);
        }
        else
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        m_indexCount = indices.size();
    }

    //  全屏四边形
    std::shared_ptr<Mesh> Mesh::FullscreenQuad()
    {
        std::vector<float> vertices =
            {-1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f, 1.0f, 1.0f, 1.0f,
             -1.0f, 1.0f, 0.0f, 1.0f};

        std::vector<uint32_t> indices =
            {0, 1, 2,
             0, 2, 3};

        GL_engine::VertexLayout vertexLayout;

        //  position
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::PositionIndex, 2, GL_FLOAT, 0});

        //  UV
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::UVIndex, 2, GL_FLOAT, sizeof(float) * 2});

        vertexLayout.stride = sizeof(float) * 4;

        auto result = std::make_shared<GL_engine::Mesh>(vertexLayout, vertices, indices);

        return result;
    }

    //  2D平面
    std::shared_ptr<Mesh> Mesh::CreatePlane2D()
    {
        std::vector<float> vertices =
            {1.0f, 1.0f,
             0.0f, 1.0f,
             0.0f, 0.0f,
             1.0f, 0.0f};

        std::vector<uint32_t> indices =
            {0, 1, 2,
             0, 2, 3};

        GL_engine::VertexLayout vertexLayout;

        //  position
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::PositionIndex, 2, GL_FLOAT, 0});

        vertexLayout.stride = sizeof(float) * 2;

        auto result = std::make_shared<GL_engine::Mesh>(vertexLayout, vertices, indices);

        return result;
    }

    //  3D创建平面
    std::shared_ptr<Mesh> Mesh::CreatePlane(float width, float height, int wSegment, int hSegment)
    {
        //  段数保护: 0 段会除零(与 box 参数化版保持一致)
        wSegment = glm::max(1, wSegment);
        hSegment = glm::max(1, hSegment);

        std::vector<float> vertices((wSegment + 1) * (hSegment + 1) * 12); //  12 floats/顶点

        const float width_half = width / 2.0f;
        const float height_half = height / 2.0f;
        const int gridX1 = wSegment + 1;
        const int gridY1 = hSegment + 1;
        const float segment_width = width / static_cast<float>(wSegment);
        const float segment_height = height / static_cast<float>(hSegment);

        for (int iy = 0; iy < gridY1; iy++)
        {
            const float y = iy * segment_height - height_half;

            for (int ix = 0; ix < gridX1; ix++)
            {
                const float x = ix * segment_width - width_half;
                const size_t vertexStart = (iy * gridX1 + ix) * 12; //  ← 8 改 12

                //  position: y 取反, 保持原朝向(第 0 行在上方)
                vertices[vertexStart + 0] = x;
                vertices[vertexStart + 1] = -y;
                vertices[vertexStart + 2] = 0.0f;

                //  normal: 面法线 +Z
                vertices[vertexStart + 3] = 0.0f;
                vertices[vertexStart + 4] = 0.0f;
                vertices[vertexStart + 5] = 1.0f;

                //  uv: 保持原有 0..1(顶部 v=1)约定
                vertices[vertexStart + 6] = ix * segment_width;           //  u 沿 +X
                vertices[vertexStart + 7] = height - iy * segment_height; //  v 沿 +Y

                //  tangent: u 轴=+X, 且与法线垂直; 手性由 v 轴(uv.y 增大方向=+Y)定 → +1
                vertices[vertexStart + 8] = 1.0f;
                vertices[vertexStart + 9] = 0.0f;
                vertices[vertexStart + 10] = 0.0f;
                vertices[vertexStart + 11] = 1.0f;
            }
        }

        //  indices: 用 uint32_t, 不再拿 float 当索引
        std::vector<uint32_t> indices;
        for (int iy = 0; iy < hSegment; iy++)
        {
            for (int ix = 0; ix < wSegment; ix++)
            {
                const uint32_t a = ix + gridX1 * iy;
                const uint32_t b = ix + gridX1 * (iy + 1);
                const uint32_t c = (ix + 1) + gridX1 * (iy + 1);
                const uint32_t d = (ix + 1) + gridX1 * iy;
                indices.push_back(a);
                indices.push_back(b);
                indices.push_back(d);
                indices.push_back(b);
                indices.push_back(c);
                indices.push_back(d);
            }
        }

        //  ---------- 顶点布局: 与 box/sphere/prism 完全一致 ----------
        GL_engine::VertexLayout vertexLayout;
        //  position
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::PositionIndex, 3, GL_FLOAT, sizeof(float) * 0});
        //  normal
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::NormalIndex, 3, GL_FLOAT, sizeof(float) * 3});
        //  uv
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::UVIndex, 2, GL_FLOAT, sizeof(float) * 6});
        //  tangent: xyz=切线, w=手性
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::TangentIndex, 4, GL_FLOAT, sizeof(float) * 8});
        vertexLayout.stride = sizeof(float) * 12;

        return std::make_shared<GL_engine::Mesh>(vertexLayout, vertices, indices);
    }

    //  3D创建箱子
    std::shared_ptr<Mesh> Mesh::CreateBox(float width, float height, float depth, float widthSegments, float heightSegments, float depthSegments)
    {
        widthSegments = glm::max(1, static_cast<int>(glm::floor(widthSegments)));
        heightSegments = glm::max(1, static_cast<int>(glm::floor(heightSegments)));
        depthSegments = glm::max(1, static_cast<int>(glm::floor(depthSegments)));

        const glm::vec3 half(width * 0.5f, height * 0.5f, depth * 0.5f);

        std::vector<float> vertices;
        std::vector<uint32_t> indices;

        //  ---------- 六个面 ----------
        //  参数: (顶点容器, 索引容器, u轴, v轴, w轴, udir, vdir, w轴坐标, 面宽, 面高, 横向段数, 纵向段数)
        //  轴映射/朝向表来自教程: 轴 0=x, 1=y, 2=z; udir/vdir 控制面的朝向与 UV 方向
        AppendPlane(vertices, indices, 2, 1, 0, -1.0f, -1.0f, +half.x, depth, height, depthSegments, heightSegments); // +X
        AppendPlane(vertices, indices, 2, 1, 0, +1.0f, -1.0f, -half.x, depth, height, depthSegments, heightSegments); // -X
        AppendPlane(vertices, indices, 0, 2, 1, +1.0f, +1.0f, +half.y, width, depth, widthSegments, depthSegments);   // +Y
        AppendPlane(vertices, indices, 0, 2, 1, +1.0f, -1.0f, -half.y, width, depth, widthSegments, depthSegments);   // -Y
        AppendPlane(vertices, indices, 0, 1, 2, +1.0f, -1.0f, +half.z, width, height, widthSegments, heightSegments); // +Z
        AppendPlane(vertices, indices, 0, 1, 2, -1.0f, -1.0f, -half.z, width, height, widthSegments, heightSegments); // -Z

        //  ---------- 顶点布局: 必须与 AppendPlane 的输出顺序一致 ----------
        GL_engine::VertexLayout vertexLayout;
        //  position
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::PositionIndex, 3, GL_FLOAT, sizeof(float) * 0});
        //  normal
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::NormalIndex, 3, GL_FLOAT, sizeof(float) * 3});
        //  uv
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::UVIndex, 2, GL_FLOAT, sizeof(float) * 6});
        //  tangent: xyz=切线方向, w=手性(±1)
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::TangentIndex, 4, GL_FLOAT, sizeof(float) * 8});
        vertexLayout.stride = sizeof(float) * 12;

        return std::make_shared<GL_engine::Mesh>(vertexLayout, vertices, indices);
    }

    //  3D创建球体
    std::shared_ptr<Mesh> Mesh::CreateSphere(float radius, int widthSegments, int heightSegments, float phiStart, float phiLength, float thetaStart, float thetaLength)
    {
        const float thetaEnd = glm::min(thetaStart + thetaLength, PI);
        int index = 0; //  顶点编号

        widthSegments = glm::max(3, widthSegments);
        heightSegments = glm::max(2, heightSegments);

        std::vector<float> vertices((widthSegments + 1) * (heightSegments + 1) * 12); //  顶点数据容器

        //  暂存计算数据
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 normal = glm::vec3(0.0f);
        glm::vec2 uv = glm::vec2(0.0f);

        std::vector<std::vector<int>> grid; //  暂存所有行编号

        // 计算 vertice normal 和 uv
        for (unsigned int iy = 0; iy <= heightSegments; iy++)
        {
            std::vector<int> verticesRow; //  暂存当前行编号

            float v = static_cast<float>(iy) / static_cast<float>(heightSegments);

            float uOffset = 0;

            if (iy == 0 && thetaStart == 0)
            {
                uOffset = 0.5f / static_cast<float>(widthSegments);
            }
            else if (iy == heightSegments && thetaEnd == PI)
            {
                uOffset = -0.5f / static_cast<float>(widthSegments);
            }
            for (unsigned int ix = 0; ix <= widthSegments; ix++)
            {
                const float u = static_cast<float>(ix) / static_cast<float>(widthSegments);

                // position
                position.x = -radius * glm::cos(phiStart + u * phiLength) * glm::sin(thetaStart + v * thetaLength);
                position.y = radius * glm::cos(thetaStart + v * thetaLength);
                position.z = radius * glm::sin(phiStart + u * phiLength) * glm::sin(thetaStart + v * thetaLength);

                size_t vertexStart = (iy * (widthSegments + 1) + ix) * 12;
                vertices[vertexStart] = position.x;
                vertices[vertexStart + 1] = position.y;
                vertices[vertexStart + 2] = position.z;

                // normal
                normal = glm::normalize(position);
                vertices[vertexStart + 3] = normal.x;
                vertices[vertexStart + 4] = normal.y;
                vertices[vertexStart + 5] = normal.z;

                // uv
                uv = glm::vec2(u + uOffset, 1 - v);
                vertices[vertexStart + 6] = uv.x;
                vertices[vertexStart + 7] = uv.y;

                //  切线: 解析导数 —— T = ∂P/∂φ 方向(=(sinφ,0,cosφ), 与 N 天然垂直, 极点也成立)
                const float phi = phiStart + u * phiLength;
                const glm::vec3 tDir(glm::sin(phi), 0.0f, glm::cos(phi));

                //  注意: uv.y = 1-v → "uv.y 增大的方向" = θ 减小方向 = -∂P/∂θ, 用它定手性
                const float theta = thetaStart + v * thetaLength;
                const glm::vec3 vDir(glm::cos(phi) * glm::cos(theta), glm::sin(theta), -glm::sin(phi) * glm::cos(theta));
                const float hand = (glm::dot(glm::cross(normal, tDir), vDir) < 0.0f) ? -1.0f : 1.0f;

                vertices[vertexStart + 8] = tDir.x;
                vertices[vertexStart + 9] = tDir.y;
                vertices[vertexStart + 10] = tDir.z;
                vertices[vertexStart + 11] = hand;

                verticesRow.push_back(index++);
            }
            grid.push_back(verticesRow);
        }

        //  生成indices
        std::vector<unsigned int> indices; //  索引数据容器
        for (unsigned int iy = 0; iy < heightSegments; iy++)
        {
            for (unsigned ix = 0; ix < widthSegments; ix++)
            {
                unsigned int a = grid[iy][ix + 1];
                unsigned int b = grid[iy][ix];
                unsigned int c = grid[iy + 1][ix];
                unsigned int d = grid[iy + 1][ix + 1];

                if (iy != 0 || thetaStart > 0)
                {
                    indices.push_back(a);
                    indices.push_back(b);
                    indices.push_back(d);
                }
                if (iy != heightSegments - 1 || thetaEnd < PI)
                {
                    indices.push_back(b);
                    indices.push_back(c);
                    indices.push_back(d);
                }
            }
        }

        GL_engine::VertexLayout vertexLayout;
        //  position
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::PositionIndex, 3, GL_FLOAT, 0});

        //  Normal
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::NormalIndex, 3, GL_FLOAT, sizeof(float) * 3});

        //  UV
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::UVIndex, 2, GL_FLOAT, sizeof(float) * 6});

        //  Tangent
        vertexLayout.elements.push_back(
            {GL_engine::VertexElement::TangentIndex, 4, GL_FLOAT, sizeof(float) * 8});

        vertexLayout.stride = sizeof(float) * 12;

        auto result = std::make_shared<GL_engine::Mesh>(vertexLayout, vertices, indices);

        return result;
    }

    //  3D创建三棱柱：底面三角形（a,b,c)+挤出向量（棱方向与长度)
    std::shared_ptr<Mesh> Mesh::CreatePrism(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec3 &extrude)
    {
        //  6 个角点: 底面三点 + 沿挤出向量平移的顶面三点
        const glm::vec3 A = a, B = b, C = c;
        const glm::vec3 A2 = a + extrude, B2 = b + extrude, C2 = c + extrude;

        //  棱柱质心: 用于自动判定每个面"朝外"的方向
        const glm::vec3 centroid = (A + B + C + A2 + B2 + C2) / 6.0f;

        std::vector<float> vertices; //  pos3 + normal3 + uv2 + tan4 = 12 floats
        std::vector<unsigned int> indices;

        //  ---------- 工具: 添加一个面(3 或 4 个点) ----------
        //  意图: 点序随便给 —— 法线统一取"背离质心"方向; 传反了自动翻转点序+法线,
        //        保证"三角形绕序(正面朝外)与法线一致"(凸体上此判定永远成立)
        auto addFace = [&](std::vector<glm::vec3> points)
        {
            glm::vec3 faceCenter(0.0f);
            for (const auto &p : points)
            {
                faceCenter += p;
            }
            faceCenter /= static_cast<float>(points.size());

            glm::vec3 normal = glm::normalize(glm::cross(points[1] - points[0], points[2] - points[0]));

            if (glm::dot(normal, faceCenter - centroid) < 0.0f)
            {
                //  法线朝内 → 翻转点序(绕序随之反转), 法线取反
                std::vector<glm::vec3> flipped(points.rbegin(), points.rend());
                points.swap(flipped);
                normal = -normal;
            }

            //  ---------- UV: 按实际几何生成 ----------
            const int count = static_cast<int>(points.size());
            glm::vec2 uv[4];

            if (count == 4)
            {
                //  侧面(挤出垂直截面时是矩形): u 沿第一条棱, v 沿挤出方向
                const float lenU = glm::length(points[1] - points[0]); //  棱长
                const float lenV = glm::length(points[3] - points[0]); //  挤出长度
                uv[0] = {0.0f, 0.0f};
                uv[1] = {lenU, 0.0f};
                uv[2] = {lenU, lenV};
                uv[3] = {0.0f, lenV};
            }
            else
            {
                //  端面(任意三角形): 保形展开 —— UV 三角形与世界三角形全等 → 零拉伸
                const glm::vec3 e1 = points[1] - points[0];
                const float lenU = glm::length(e1);
                const glm::vec3 uDir = (lenU > 1e-6f) ? (e1 / lenU) : glm::vec3(1.0f, 0.0f, 0.0f);
                const glm::vec3 e2 = points[2] - points[0];
                const float x2 = glm::dot(e2, uDir);                                    //  c 在 u 轴上的投影
                const float y2 = glm::sqrt(glm::max(glm::dot(e2, e2) - x2 * x2, 0.0f)); //  c 到 u 轴的垂距
                uv[0] = {0.0f, 0.0f};
                uv[1] = {lenU, 0.0f};
                uv[2] = {x2, y2};
            }

            //  ---------- 切线: 与 UV 同一套方向 ----------
            //  意图: 面是平的 → u 方向天然垂直于面法线; 手性由 v 方向定
            //  退化保护: 边长为 0 / 三角形退化时退回正交兜底方向, 避免 normalize(0) → NaN 扩散
            const glm::vec3 e1 = points[1] - points[0];
            const float lenT = glm::length(e1);
            const glm::vec3 tDir = (lenT > 1e-6f) ? (e1 / lenT) : glm::vec3(1.0f, 0.0f, 0.0f);

            glm::vec3 vDir;
            if (count == 4)
            {
                const glm::vec3 ev = points[3] - points[0]; //  四边形: 第三条边方向 = v 轴
                const float lenV = glm::length(ev);
                vDir = (lenV > 1e-6f) ? (ev / lenV) : glm::cross(normal, tDir);
            }
            else
            {
                const glm::vec3 e2 = points[2] - points[0]; //  三角形: v 轴 = e2 去掉 u 分量后的垂直方向
                const glm::vec3 perp = e2 - tDir * glm::dot(e2, tDir);
                const float lenP = glm::length(perp);
                vDir = (lenP > 1e-6f) ? (perp / lenP) : glm::cross(normal, tDir);
            }
            const float hand = (glm::dot(glm::cross(normal, tDir), vDir) < 0.0f) ? -1.0f : 1.0f;

            //  ---------- 顶点: pos3 + normal3 + uv2 + tan4 ----------
            const unsigned int base = static_cast<unsigned int>(vertices.size() / 12);
            for (int i = 0; i < count; ++i)
            {
                vertices.insert(vertices.end(), {points[i].x, points[i].y, points[i].z,
                                                 normal.x, normal.y, normal.z,
                                                 uv[i].x, uv[i].y,
                                                 tDir.x, tDir.y, tDir.z, hand});
            }

            //  索引: 三角形扇
            indices.insert(indices.end(), {base, base + 1, base + 2});
            if (count == 4)
            {
                indices.insert(indices.end(), {base, base + 2, base + 3});
            }
        };

        //  ---------- 两个端面 ----------
        addFace({A, B, C});
        addFace({A2, B2, C2});

        //  ---------- 三个侧面(四边形: 底面棱 + 对应顶面棱) ----------
        addFace({A, B, B2, A2});
        addFace({B, C, C2, B2});
        addFace({C, A, A2, C2});

        GL_engine::VertexLayout vertexLayout;
        //  position
        vertexLayout.elements.push_back({GL_engine::VertexElement::PositionIndex, 3, GL_FLOAT, 0});
        //  normal
        vertexLayout.elements.push_back({GL_engine::VertexElement::NormalIndex, 3, GL_FLOAT, sizeof(float) * 3});
        //  uv
        vertexLayout.elements.push_back({GL_engine::VertexElement::UVIndex, 2, GL_FLOAT, sizeof(float) * 6});
        //  tangent
        vertexLayout.elements.push_back({GL_engine::VertexElement::TangentIndex, 4, GL_FLOAT, sizeof(float) * 8});
        vertexLayout.stride = sizeof(float) * 12;

        auto result = std::make_shared<GL_engine::Mesh>(vertexLayout, vertices, indices);

        return result;
    }

    //  意图: 单面构建逻辑复用 6 次; u/v/w = 轴映射(0=x,1=y,2=z), *dir = 朝向, facePos = 该面在 w 轴的坐标
    //  输出统一布局: pos3 + nrm3 + uv2 + tan4(切线解析可得, 无需累加算法)
    void Mesh::AppendPlane(std::vector<float> &vertices, std::vector<uint32_t> &indices, int u, int v, int w, float udir, float vdir, float facePos, float width, float height, int gridX, int gridY)
    {
        const uint32_t start = static_cast<uint32_t>(vertices.size() / 12); //  12 floats/顶点
        const float segW = width / static_cast<float>(gridX);
        const float segH = height / static_cast<float>(gridY);

        for (int iy = 0; iy <= gridY; ++iy)
        {
            for (int ix = 0; ix <= gridX; ++ix)
            {
                const float localU = ix * segW; //  0..width
                const float localV = iy * segH; //  0..height

                glm::vec3 pos(0.0f), nrm(0.0f), tan(0.0f), bit(0.0f);
                pos[u] = (localU - width * 0.5f) * udir;
                pos[v] = (localV - height * 0.5f) * vdir;
                pos[w] = facePos;
                nrm[w] = (facePos > 0.0f) ? 1.0f : -1.0f;

                //  切线: 解析可得 —— T 沿 u 轴, 副切线沿 v 轴(只存 T + 手性)
                tan[u] = udir;
                bit[v] = vdir;
                //  UV: 世界尺寸; v 从底部向上增大(v=height 在顶) —— 与平面/球体/stbi 翻转约定统一
                //  意图: 垂直面不再上下镜像(原来 v=0 在顶, 墙上的贴图相对平面是倒的)
                const glm::vec2 uv(localU, height - localV);

                const float hand = (glm::dot(glm::cross(nrm, tan), -bit) < 0.0f) ? -1.0f : 1.0f;

                vertices.insert(vertices.end(), {pos.x, pos.y, pos.z,
                                                 nrm.x, nrm.y, nrm.z,
                                                 uv.x, uv.y,
                                                 tan.x, tan.y, tan.z, hand});
            }
        }

        //  索引(注意用 uint32_t)
        for (int iy = 0; iy < gridY; ++iy)
        {
            for (int ix = 0; ix < gridX; ++ix)
            {
                const uint32_t a = start + ix + (gridX + 1) * iy;
                const uint32_t b = a + (gridX + 1);
                const uint32_t c = b + 1;
                const uint32_t d = a + 1;
                indices.insert(indices.end(), {a, b, d, b, c, d});
            }
        }
    }
}