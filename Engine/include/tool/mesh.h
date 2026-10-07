#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <tool/shader.h>

#include <string>
#include <vector>

using namespace std;

#ifndef BUFFER_GROMETRY
struct Vertex
{
    glm::vec3 Position;  // 顶点属性
    glm::vec3 Normal;    // 法线
    glm::vec2 TexCoords; // 纹理坐标

    // 切线空间属性
    glm::vec3 Tangent;   // 切线
    glm::vec3 Bitangent; // 副切线
};
#endif

struct Texture
{
    unsigned int id;
    string type;
    string path;
};

class Mesh
{
public:
    // 网格数据
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO;

    //  构造函数
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // 已获取所有必要数据，接下来设置顶点缓冲区及其属性指针
        setupMesh();
    }

    // 渲染网格
    void Draw(Shader &shader)
    {
        // 绑定合适的纹理
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // 绑定前激活适当的纹理单元
            // 获取纹理序号（diffuse_textureN 中的 N）
            string number;
            string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);
            else if (name == "texture_normal")
                number = std::to_string(normalNr++);
            else if (name == "texture_height")
                number = std::to_string(heightNr++);

            // 设置纹理单元
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            // 最后绑定纹理
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // 绘制网格
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // 将内容设置回默认值
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // 渲染数据
    unsigned int VBO, EBO;

    void setupMesh()
    {
        // 设置VAO、VBO、EBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // 加载数据进顶点缓冲区
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        /*
        结构的一个优点是，它们的内存布局都是顺序的。
        如果我们将结构体作为一个数据数组使用，那么它将会以顺序排列结构体的变量，这将会直接转换为我们在数组缓冲中所需要的float（实际上是字节）数组
        */
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // 设置顶点属性指针
        // 顶点位置
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        // 顶点法向量
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
        // 顶点纹理坐标
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
        // 顶点切线
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
        // 顶点副切线
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }
};

#endif