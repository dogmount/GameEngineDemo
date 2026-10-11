#include "scene/gameObject.h"
#include "engine/Engine.h"
#include "shaderAPI/vertexLayout.h"
#include "shaderAPI/texture.h"
#include "render/material.h"
#include "render/mesh.h"
#include "scene/components/meshComponent.h"
#include "scene/components/animationComponent.h"

#include <vector>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>

namespace GL_engine
{
    //  加载属性
    void GameObject::LoadProperties(const nlohmann::json &json)
    {
    }

    //  初始化
    void GameObject::Init()
    {
    }

    //  更新
    void GameObject::Update(float deltaTime)
    {
        if (!m_active)
        {
            return;
        }

        //  更新对象本身的组件
        for (auto &component : m_components)
        {
            component->Update(deltaTime);
        }

        //  更新对象子对象
        for (auto it = m_children.begin(); it != m_children.end();)
        {
            if ((*it)->IsAlive())
            {
                (*it)->Update(deltaTime);
                ++it;
            }
            else
            {
                it = m_children.erase(it);
            }
        }
    }

    //  设置/获取对象名
    void GameObject::SetName(const std::string &name)
    {
        m_name = name;
    }
    const std::string &GameObject::GetName() const
    {
        return m_name;
    }

    //  设置/获取父对象
    bool GameObject::SetParent(GameObject *parent)
    {
        if (!m_scene)
        {
            return false;
        }

        return m_scene->SetParent(this, parent);
    }
    GameObject *GameObject::GetParent()
    {
        return m_parent;
    }

    //  获取所属场景
    Scene *GameObject::GetScene()
    {
        return m_scene;
    }

    //  设置对象销毁
    void GameObject::MarkForDestroy()
    {
        m_isAlive = false;
    }

    //  获取对象存活状态
    bool GameObject::IsAlive() const
    {
        return m_isAlive;
    }

    //  添加组件
    void GameObject::AddComponent(Component *component)
    {
        if (!component)
        {
            return;
        }
        //  emplace_back接收裸指针component，就地构造std::unique_ptr
        m_components.emplace_back(component);
        component->m_owner = this;
        component->Init();
    }

    //  设置/获取对象激活状态
    void GameObject::SetActive(bool active)
    {
        m_active = active;
    }
    bool GameObject::IsActive() const
    {
        return m_active;
    }

    //  查找对象
    GameObject *GameObject::FindChildByName(const std::string &name)
    {
        if (m_name == name)
        {
            return this;
        }

        for (auto &child : m_children)
        {
            if (auto res = child->FindChildByName(name))
            {
                return res;
            }
        }

        return nullptr;
    }

    //  获取子对象
    const std::vector<std::unique_ptr<GameObject>> &GameObject::GetChildren() const
    {
        return m_children;
    }

    //  设置/获取对象位置数据
    void GameObject::SetPosition(const glm::vec3 &pos)
    {
        m_position = pos;
    }
    const glm::vec3 &GameObject::GetPosition() const
    {
        return m_position;
    }

    //  设置/获取世界位置
    void GameObject::SetWorldPosition(const glm::vec3 &pos)
    {
        if (m_parent)
        {
            glm::mat4 parentWorld = m_parent->GetWorldTransform();
            glm::mat4 invParentWorld = glm::inverse(parentWorld);
            glm::vec4 localPos = invParentWorld * glm::vec4(pos, 1.0f);
            SetPosition(glm::vec3(localPos) / localPos.w);
        }
        else
        {
            SetPosition(pos);
        }
    }
    glm::vec3 GameObject::GetWorldPosition() const
    {
        glm::vec4 hom = GetWorldTransform() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return (glm::vec3(hom) / hom.w);
    }

    glm::vec2 GameObject::GetWorldPosition2D() const
    {
        glm::vec4 hom = GetWorldTransform2D() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return (glm::vec3(hom) / hom.w);
    }

    //  设置/获取2D位置
    void GameObject::SetPosition2D(const glm::vec2 &pos)
    {
        m_position = glm::vec3(pos, 0.0f);
    }
    glm::vec2 GameObject::GetPosition2D() const
    {
        return glm::vec2(m_position);
    }

    //  改进点：设置/获取旋转（输入欧拉角）
    void GameObject::SetRotationEulerYXZ(const glm::vec3 eulerDegrees)
    {
        // 归一化角度到 [-180, 180]，防止 UI 数值越界
        glm::vec3 normalized = glm::vec3(
            fmod(eulerDegrees.x + 180.0f, 360.0f) - 180.0f,
            fmod(eulerDegrees.y + 180.0f, 360.0f) - 180.0f,
            fmod(eulerDegrees.z + 180.0f, 360.0f) - 180.0f);

        glm::vec3 rad = glm::radians(eulerDegrees);
        glm::quat qY = glm::angleAxis(rad.y, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat qX = glm::angleAxis(rad.x, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat qZ = glm::angleAxis(rad.z, glm::vec3(0.0f, 0.0f, 1.0f));
        m_rotation = qY * qX * qZ; // 矩阵乘法顺序：先 Z，再 X，再 Y（外部欧拉角顺序）
    }
    glm::vec3 GameObject::GetRotationEulerYXZ() const
    {
        glm::mat4 rotationMatrix = glm::mat4_cast(m_rotation);
        float yaw, pitch, roll;
        glm::extractEulerAngleYXZ(rotationMatrix, yaw, pitch, roll); // 得到的 yaw, pitch, roll 就是 Y-X-Z 顺序的欧拉角（弧度）

        return glm::degrees(glm::vec3(pitch, yaw, roll));
    }

    //  设置/获取对象旋转数据
    void GameObject::SetRotation(const glm::quat &rot)
    {
        //  注意：归一化只是"保命"——把 (30,20,50,1) 归一化后的朝向并不是你想要的 30°/20°/50°，所以要同时修场景数据（用 rotationEuler）。
        const float lenSq = glm::dot(rot, rot);
        m_rotation = (lenSq > 1e-12f) ? glm::normalize(rot) : glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }
    const glm::quat &GameObject::GetRotation() const
    {
        return m_rotation;
    }

    //  设置/获取世界旋转
    void GameObject::SetWorldRotation(const glm::quat &rot)
    {
        if (m_parent)
        {
            glm::quat parentWorldRot = m_parent->GetWorldRotation();
            glm::quat invParentWorldRot = glm::inverse(parentWorldRot);
            glm::quat newLocalRot = invParentWorldRot * rot;
            SetRotation(newLocalRot);
        }
        else
        {
            SetRotation(rot);
        }
    }
    glm::quat GameObject::GetWorldRotation()
    {
        if (m_parent)
        {
            return m_parent->GetWorldRotation() * m_rotation;
        }
        else
        {
            return m_rotation;
        }
    }

    //  设置/获取2D旋转
    void GameObject::SetRotation2D(float rotation)
    {
        m_rotation = glm::angleAxis(rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    float GameObject::GetRotation2D() const
    {
        return glm::angle(m_rotation);
    }

    //  设置/获取对象缩放数据
    void GameObject::SetScale(const glm::vec3 &scale)
    {
        m_scale = scale;
    }
    const glm::vec3 &GameObject::GetScale() const
    {
        return m_scale;
    }

    //  设置/获取2D缩放
    void GameObject::SetScale2D(const glm::vec2 &scale)
    {
        m_scale = glm::vec3(scale, 1.0f);
    }
    glm::vec2 GameObject::GetScale2D() const
    {
        return glm::vec2(m_scale);
    }

    //  获取2D局部变换
    glm::mat4 GameObject::GetLocalTransform2D() const
    {
        glm::mat4 mat = glm::mat4(1.0f);

        const auto rotationZ = GetRotation2D();
        float c = cos(rotationZ);
        float s = sin(rotationZ);

        mat[0][0] = m_scale.x * c;
        mat[0][1] = m_scale.x * s;

        mat[1][0] = -m_scale.y * s;
        mat[1][1] = m_scale.y * c;

        mat[3][0] = m_position.x;
        mat[3][1] = m_position.y;

        return mat;
    }

    //  获取2D世界变换
    glm::mat4 GameObject::GetWorldTransform2D() const
    {
        if (m_parent)
        {
            return m_parent->GetWorldTransform2D() * GetLocalTransform2D();
        }
        else
        {
            return GetLocalTransform2D();
        }
    }

    //  获取局部变换
    glm::mat4 GameObject::GetLocalTransform() const
    {
        glm::mat4 mat = glm::mat4(1.0f);

        //  平移变换
        mat = glm::translate(mat, m_position);

        //  旋转变换(四元数)
        mat = mat * glm::mat4_cast(m_rotation); //  将四元数转化为矩阵

        //  缩放变换
        mat = glm::scale(mat, m_scale);

        return mat;
    }

    //  获取世界变换
    glm::mat4 GameObject::GetWorldTransform() const
    {
        //  有父对象
        if (m_parent)
        {
            return m_parent->GetWorldTransform() * GetLocalTransform();
        }
        else
        {
            return GetLocalTransform();
        }
    }

    //  辅助函数---glb内嵌纹理key生成函数
    std::string MakeEmbeddedTextureKey(const std::filesystem::path &folder, cgltf_size offset, cgltf_size size)
    {
        return folder.string() + "/embedded_" +
               std::to_string(offset) + "_" +
               std::to_string(size);
    }

    //  LoadGLTF的辅助函数
    void ParaseGLTFNode(cgltf_node *node, GameObject *parent, const std::filesystem::path &folder)
    {
        //  辅助函数1. 通过访问器读取float数据
        auto readFloats = [](const cgltf_accessor *acc, cgltf_size i, float *out, int n)
        {
            std::fill(out, out + n, 0.0f);                         //  保护性初始化
            return cgltf_accessor_read_float(acc, i, out, n) == 1; //  cgltf_accessor_read_xxx（cgltf访问器，访问索引，输出缓冲区指针，读取数量）成功返回1，失败返回0
        };

        //  辅助函数2. 通过访问器读取索引数据
        auto readIndex = [](const cgltf_accessor *acc, cgltf_size i)
        {
            cgltf_uint out = 0;
            cgltf_bool ok = cgltf_accessor_read_uint(acc, i, &out, 1);
            return ok ? static_cast<uint32_t>(out) : 0;
        };

        //  辅助函数3. 读取gltf纹理
        //  统一加载 gltf 纹理(外部 uri / 内嵌 bufferView)
        auto LoadGltfTexture = [&](cgltf_texture *texture) -> std::shared_ptr<Texture>
        {
            if (!texture || !texture->image)
            {
                return nullptr;
            }
            if (texture->image->uri) //  外部文件
            {
                auto path = folder / std::string(texture->image->uri);
                return Engine::GetInstance().GetTextureManager().GetOrLoadTexture(path.string());
            }
            if (texture->image->buffer_view) //  内嵌
            {
                auto view = texture->image->buffer_view;
                if (view->buffer && view->buffer->data)
                {
                    std::string key = MakeEmbeddedTextureKey(folder, view->offset, view->size);
                    const void *data = static_cast<const uint8_t *>(view->buffer->data) + view->offset;
                    return Engine::GetInstance().GetTextureManager().GetOrLoadEmbeddedTexture(key, data, view->size);
                }
            }
            return nullptr;
        };

        //  创建游戏对象
        auto object = parent->GetScene()->CreateObject(node->name, parent);

        //  步骤1、处理节点矩阵
        if (node->has_matrix) // 节点为统一矩阵（matrix）
        {
            auto mat = glm::make_mat4(node->matrix);
            glm::vec3 translation, scale, skew;
            glm::vec4 perspective;
            glm::quat orientation;
            glm::decompose(mat, scale, orientation, translation, skew, perspective); // glm::decompose 将矩阵分解为位移、旋转、缩放等矩阵

            object->SetPosition(translation);
            object->SetRotation(orientation);
            object->SetScale(scale);
        }
        else // 节点为独立位移translation、旋转rotation、缩放scale矩阵
        {
            if (node->has_translation)
            {
                object->SetPosition(
                    glm::vec3(node->translation[0], node->translation[1], node->translation[2]));
            }

            if (node->has_rotation)
            {
                object->SetRotation(
                    glm::quat(node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]));
            }

            if (node->has_scale)
            {
                object->SetScale(
                    glm::vec3(node->scale[0], node->scale[1], node->scale[2]));
            }
        }

        //  步骤2、处理节点网格
        if (node->mesh)
        {
            for (cgltf_size pi = 0; pi < node->mesh->primitives_count; ++pi)
            {
                //  创建网格
                std::shared_ptr<Mesh> objectMesh;

                auto &primitive = node->mesh->primitives[pi];
                if (primitive.type != cgltf_primitive_type_triangles) // 跳过非三角图元类型
                {
                    continue;
                }

                VertexLayout vertexLayout;
                cgltf_accessor *accessors[4] = {nullptr, nullptr, nullptr, nullptr};
                for (cgltf_size ai = 0; ai < primitive.attributes_count; ++ai) // 遍历顶点属性
                {
                    auto &attr = primitive.attributes[ai];
                    auto acc = attr.data;
                    if (!acc) // 防御性编程，若可选属性未定义或文件损坏，则跳过
                    {
                        continue;
                    }

                    VertexElement element;
                    element.type = GL_FLOAT;

                    switch (attr.type)
                    {
                    case cgltf_attribute_type_position:
                    {
                        accessors[VertexElement::PositionIndex] = acc; // 记录访问器地址
                        element.index = VertexElement::PositionIndex;
                        element.size = 3;
                    }
                    break;
                    case cgltf_attribute_type_color:
                    {
                        if (attr.index != 0) //  默认使用索引为0的颜色通道
                        {
                            continue;
                        }
                        accessors[VertexElement::ColorIndex] = acc; // 记录访问器地址
                        element.index = VertexElement::ColorIndex;
                        element.size = 3;
                    }
                    break;
                    case cgltf_attribute_type_texcoord: // 改进点：未来可支持加载多个纹理，如法线贴图等
                    {
                        if (attr.index != 0) //  默认使用索引为0的纹理通道
                        {
                            continue;
                        }
                        accessors[VertexElement::UVIndex] = acc; // 记录访问器地址
                        element.index = VertexElement::UVIndex;
                        element.size = 2;
                    }
                    break;
                    case cgltf_attribute_type_normal:
                    {
                        accessors[VertexElement::NormalIndex] = acc; // 记录访问器地址
                        element.index = VertexElement::NormalIndex;
                        element.size = 3;
                    }
                    break;
                    default:
                        continue;
                    }

                    if (element.size > 0)
                    {
                        element.offset = vertexLayout.stride;
                        vertexLayout.stride += element.size * sizeof(float);
                        vertexLayout.elements.push_back(element);
                    }
                }

                //  访问器未找到顶点数据，访问下一个图元
                if (!accessors[VertexElement::PositionIndex])
                {
                    continue;
                }

                //  1.获取顶点数量
                auto vertexCount = accessors[VertexElement::PositionIndex]->count;
                std::vector<float> vertices;
                vertices.resize((vertexLayout.stride / sizeof(float)) * vertexCount); //  设置顶点缓冲区大小

                //  处理图元顶点数据
                for (cgltf_size vi = 0; vi < vertexCount; ++vi)
                {
                    for (auto &el : vertexLayout.elements)
                    {
                        if (!accessors[el.index])
                        {
                            continue;
                        }

                        auto index = (vi * vertexLayout.stride + el.offset) / sizeof(float); //  设置vertices索引
                        float *outData = &vertices[index];
                        readFloats(accessors[el.index], vi, outData, el.size); //  （访问器地址，顶点数据索引，vertices对应数据位置指针，读取数据个数）

                        //  修复: glTF 的 UV 原点在【左上角】(v 向下增大),
                        //        而 Texture::Load 默认做了 stbi 垂直翻转(v 向上约定) →
                        //        导入时把 v 翻回来, 否则 gltf 模型贴图上下颠倒
                        if (el.index == VertexElement::UVIndex && el.size == 2)
                        {
                            outData[1] = 1.0f - outData[1];
                        }
                    }
                }

                //  2.处理图元索引数据
                if (primitive.indices)
                {
                    auto indexCount = primitive.indices->count;
                    std::vector<uint32_t> indices(indexCount);
                    for (cgltf_size i = 0; i < indexCount; ++i)
                    {
                        indices[i] = readIndex(primitive.indices, i);
                    }
                    objectMesh = std::make_shared<Mesh>(vertexLayout, vertices, indices);
                }
                else
                {
                    //  图元无EBO数据
                    objectMesh = std::make_shared<Mesh>(vertexLayout, vertices);
                }

                auto mat = std::make_shared<Material>();
                mat->SetShaderProgram(Engine::GetInstance().GetShaderAPI().GetDefaultProgram());

                //  3.处理图元材质数据
                mat->SetFloat("uHasNormalMap", 0.0f);
                mat->SetFloat("uHasRoughnessMap", 0.0f);

                //  3.处理图元材质数据
                if (primitive.material)
                {
                    auto gltfMat = primitive.material;
                    if (gltfMat->has_pbr_metallic_roughness) // 图元材质有pbr金属粗糙度
                    {
                        auto pbr = gltfMat->pbr_metallic_roughness;
                        auto texture = pbr.base_color_texture.texture;
                        if (texture && texture->image) //  贴图存在且有图像
                        {
                            if (auto tex = LoadGltfTexture(gltfMat->pbr_metallic_roughness.base_color_texture.texture))
                            {
                                mat->SetTexture("baseColorTexture", tex);
                            }

                            //  金属粗糙度(glTF 规范: G=粗糙度, B=金属度; R 是 AO, 暂未用)
                            if (auto tex = LoadGltfTexture(gltfMat->pbr_metallic_roughness.metallic_roughness_texture.texture))
                            {
                                mat->SetTexture("metallicRoughnessTexture", tex);
                                mat->SetFloat("uHasRoughnessMap", 1.0f);
                            }
                        }
                    }
                    else if (gltfMat->has_pbr_specular_glossiness) //  图元材质有pbr高光光泽度
                    {
                        if (auto tex = LoadGltfTexture(gltfMat->pbr_specular_glossiness.diffuse_texture.texture))
                        {
                            mat->SetTexture("baseColorTexture", tex);
                        }
                    }

                    //  法线贴图(gltf 核心字段, 与上面用哪个 pbr 分支无关)
                    if (auto tex = LoadGltfTexture(gltfMat->normal_texture.texture))
                    {
                        mat->SetTexture("normalMap", tex);
                        mat->SetFloat("uHasNormalMap", 1.0f);
                    }
                }
                object->AddComponent(new MeshComponent(mat, objectMesh));
            }
        }

        //  步骤3、处理子节点
        for (cgltf_size ci = 0; ci < node->children_count; ++ci)
        {
            ParaseGLTFNode(node->children[ci], object, folder);
        }
    }

    //  辅助函数：读取标量
    auto ReadScalar = [](cgltf_accessor *acc, cgltf_size index)
    {
        float res = 0.0f;
        cgltf_accessor_read_float(acc, index, &res, 1);
        return res;
    };

    //  辅助函数：读取vec3
    auto ReadVec3 = [](cgltf_accessor *acc, cgltf_size index)
    {
        glm::vec3 res;
        cgltf_accessor_read_float(acc, index, glm::value_ptr(res), 3);
        return res;
    };

    //  辅助函数：读取四元数
    auto ReadQuat = [](cgltf_accessor *acc, cgltf_size index)
    {
        float res[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        cgltf_accessor_read_float(acc, index, res, 4);
        return glm::quat(res[3], res[0], res[1], res[2]);
    };

    auto ReadTimes = [](cgltf_accessor *acc, std::vector<float> &outTimes)
    {
        outTimes.resize(acc->count);
        for (cgltf_size i = 0; i < acc->count; ++i)
        {
            outTimes[i] = ReadScalar(acc, i);
        }
    };

    auto ReadOutputVec3 = [](cgltf_accessor *acc, std::vector<glm::vec3> &outValues)
    {
        outValues.resize(acc->count);
        for (cgltf_size i = 0; i < acc->count; ++i)
        {
            outValues[i] = ReadVec3(acc, i);
        }
    };

    auto ReadOutputQuat = [](cgltf_accessor *acc, std::vector<glm::quat> &outValues)
    {
        outValues.resize(acc->count);
        for (cgltf_size i = 0; i < acc->count; ++i)
        {
            outValues[i] = ReadQuat(acc, i);
        }
    };

    //  加载GLTF
    GameObject *GameObject::LoadGLTF(const std::string &path, Scene *gameScene)
    {
        auto contents = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);
        if (contents.empty())
        {
            std::cout << ".gltf File is empty !!!" << std::endl;
            return nullptr;
        }

        if (!gameScene)
        {
            std::cout << "Scene of GameObject is empty !!!" << std::endl;
            return nullptr;
        }

        cgltf_options options = {}; // cgltf配置选项结构体
        cgltf_data *data = nullptr; // 数据缓冲指针

        //  1、将数据结构解析结果放入data
        cgltf_result res = cgltf_parse(&options, contents.data(), contents.size(), &data); //  cgltf_parse(&options结构体，数据指针，数据大小，&输出缓冲区指针)
        if (res != cgltf_result_success)
        {
            std::cout << "cgltf_parse is Failed !!!" << std::endl;
            return nullptr;
        }

        auto fullPath = Engine::GetInstance().GetFileSystem().GetAssetsFolder() / path;
        auto fullFolderPath = fullPath.remove_filename();
        auto relativeFolderPath = std::filesystem::path(path).remove_filename();

        //  2、根据fullPath路径获取数据，填入data->buffers[i].data
        res = cgltf_load_buffers(&options, data, fullFolderPath.string().c_str()); //  cgltf_load_buffers(&options结构体，输出缓冲区指针，文件路径)
        if (res != cgltf_result_success)
        {
            std::cout << "cgltf_load_buffers is Failed !!!" << std::endl;
            cgltf_free(data);
            return nullptr;
        }

        // auto scene = &data->scenes[0];
        // auto scene = &data->scene[0];

        //  3、创建对象保存gltf文件中的场景
        auto resultObject = gameScene->CreateObject("Result");
        cgltf_scene *scene = nullptr;
        if (data->scenes_count > 0 && data->scenes != nullptr)
        {
            scene = &data->scenes[0];
        }
        else
        {
            std::cout << "cgltf_data_scene is NULL !!!" << std::endl;
            cgltf_free(data);
            return nullptr;
        }

        for (cgltf_size i = 0; i < scene->nodes_count; ++i)
        {
            auto node = scene->nodes[i];
            ParaseGLTFNode(node, resultObject, relativeFolderPath);
        }

        //  4、解析动画
        std::vector<std::shared_ptr<AnimationClip>> clips;
        for (cgltf_size ai = 0; ai < data->animations_count; ++ai)
        {
            auto &anim = data->animations[ai];

            auto clip = std::make_shared<AnimationClip>();
            clip->name = anim.name ? anim.name : "noname";
            clip->duration = 0.0f;

            std::unordered_map<cgltf_node *, size_t> trackIndexOf;

            //  辅助函数：获取或创建动画轨道(避免重复创建动画轨道)
            auto GetOrCreateTrack = [&](cgltf_node *node) -> TransformTrack &
            {
                auto it = trackIndexOf.find(node);
                if (it != trackIndexOf.end())
                {
                    return clip->tracks[it->second];
                }

                TransformTrack track;
                track.targetName = node->name;
                clip->tracks.push_back(track);
                size_t idx = clip->tracks.size() - 1;
                trackIndexOf[node] = idx;
                return clip->tracks[idx];
            };

            //  遍历通道
            for (cgltf_size ci = 0; ci < anim.channels_count; ++ci)
            {
                auto &channel = anim.channels[ci];
                auto sampler = channel.sampler;

                if (!channel.target_node || !sampler || !sampler->input || !sampler->output)
                {
                    continue;
                }

                std::vector<float> times;
                ReadTimes(sampler->input, times); //  获取时间轴

                auto &track = GetOrCreateTrack(channel.target_node);

                switch (channel.target_path)
                {
                case cgltf_animation_path_type_translation:
                {
                    std::vector<glm::vec3> values;
                    ReadOutputVec3(sampler->output, values);
                    track.positions.resize(times.size());
                    for (size_t i = 0; i < times.size(); ++i)
                    {
                        track.positions[i].time = times[i];
                        track.positions[i].value = values[i];
                    }
                }
                break;
                case cgltf_animation_path_type_rotation:
                {
                    std::vector<glm::quat> values;
                    ReadOutputQuat(sampler->output, values);
                    track.rotations.resize(times.size());
                    for (size_t i = 0; i < times.size(); ++i)
                    {
                        track.rotations[i].time = times[i];
                        track.rotations[i].value = values[i];
                    }
                }
                break;
                case cgltf_animation_path_type_scale:
                {
                    std::vector<glm::vec3> values;
                    ReadOutputVec3(sampler->output, values);
                    track.scales.resize(times.size());
                    for (size_t i = 0; i < times.size(); ++i)
                    {
                        track.scales[i].time = times[i];
                        track.scales[i].value = values[i];
                    }
                }
                break;
                default:
                    break;
                }

                clip->duration = std::max(clip->duration, times.back());
            }

            clips.push_back(std::move(clip));
        }

        if (!clips.empty())
        {
            auto animComp = new AnimationComponent();
            resultObject->AddComponent(animComp);
            for (auto &clip : clips)
            {
                animComp->RegisterClip(clip->name, clip);
            }
        }

        cgltf_free(data);

        return resultObject;
    }

    //  获取游戏对象工厂实例
    GameObjectFactory &GameObjectFactory::GetInstance()
    {
        static GameObjectFactory instance;
        return instance;
    }

    //  调用对应类型游戏对象创建器
    GameObject *GameObjectFactory::GameObjectCreate(const std::string &typeName)
    {
        auto it = m_creators.find(typeName);
        if (it == m_creators.end())
        {
            return nullptr;
        }

        return it->second->CreateGameObject();
    }
}