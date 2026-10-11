#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "scene/component.h"

namespace GL_engine
{
    class Scene;

    class GameObject
    {
    protected:
        GameObject() = default;

    public:
        virtual ~GameObject() = default;

        virtual void LoadProperties(const nlohmann::json &json); //  加载属性
        virtual void Init();                                     //  初始化
        virtual void Update(float deltaTime);                    //  更新

        //  设置/获取对象名
        void SetName(const std::string &name);
        const std::string &GetName() const;

        //  设置/获取父对象
        bool SetParent(GameObject *parent);
        GameObject *GetParent();

        //  获取所属场景
        Scene *GetScene();

        //  设置对象销毁
        void MarkForDestroy();

        //  获取对象存活状态
        bool IsAlive() const;

        //  添加组件
        void AddComponent(Component *component);

        //  设置/获取动画激活状态
        void SetActive(bool active);
        bool IsActive() const;

        //  模板获取组件
        template <typename T, typename = typename std::enable_if_t<std::is_base_of_v<Component, T>>>
        T *GetComponent()
        {
            //  获取并更新组件id
            size_t typeId = Component::StaticTypeId<T>();

            for (auto &component : m_components)
            {
                if (component->GetTypeId() == typeId || ComponentFactory::GetInstance().HasParent(component->GetTypeId(), typeId))
                {
                    return static_cast<T *>(component.get());
                }
            }

            return nullptr;
        }

        //  查找对象
        GameObject *FindChildByName(const std::string &name);

        //  获取子对象数组
        const std::vector<std::unique_ptr<GameObject>> &GetChildren() const;

        //  设置/获取对象位置数据
        void SetPosition(const glm::vec3 &pos);
        const glm::vec3 &GetPosition() const;

        //  设置/获取世界位置
        void SetWorldPosition(const glm::vec3 &pos);
        glm::vec3 GetWorldPosition() const;

        //  设置/获取2D世界位置
        // void SetWorldPosition2D(const glm::vec2 &pos);
        glm::vec2 GetWorldPosition2D() const;

        //  设置/获取2D位置
        void SetPosition2D(const glm::vec2 &pos);
        glm::vec2 GetPosition2D() const;

        //  改进点：设置/获取旋转（欧拉角y-x-z）
        void SetRotationEulerYXZ(const glm::vec3 eulerDegrees);
        glm::vec3 GetRotationEulerYXZ() const;

        //  设置/获取对象旋转数据
        void SetRotation(const glm::quat &rot);
        const glm::quat &GetRotation() const;

        //  设置/获取世界旋转
        void SetWorldRotation(const glm::quat &rot);
        glm::quat GetWorldRotation();

        //  设置/获取2D旋转
        void SetRotation2D(float rotation);
        float GetRotation2D() const;

        //  设置/获取对象缩放数据
        void SetScale(const glm::vec3 &scale);
        const glm::vec3 &GetScale() const;

        //  设置/获取2D缩放
        void SetScale2D(const glm::vec2 &scale);
        glm::vec2 GetScale2D() const;

        //  获取2D局部变换
        glm::mat4 GetLocalTransform2D() const;

        //  获取2D世界变换
        glm::mat4 GetWorldTransform2D() const;

        //  获取局部变换
        glm::mat4 GetLocalTransform() const;

        //  获取世界变换
        glm::mat4 GetWorldTransform() const;

        //  加载GLTF
        static GameObject *LoadGLTF(const std::string &path, Scene *scene);

    protected:
        friend class Scene;

        std::string m_name;                                   //  对象名
        GameObject *m_parent = nullptr;                       //  父对象指针
        Scene *m_scene = nullptr;                             //  对象所处场景指针
        std::vector<std::unique_ptr<GameObject>> m_children;  //  子对象指针数组
        std::vector<std::unique_ptr<Component>> m_components; //  对象组件指针数组
        bool m_isAlive = true;                                //  对象存活状态

        glm::vec3 m_position = glm::vec3(0.0f);                   //  对象位置
        glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); //  对象旋转(四元数实现)
        glm::vec3 m_scale = glm::vec3(1.0f);                      //  对象缩放

        //  对象激活状态设置
        bool m_active = true;
    };

    //  游戏对象构造器基类
    class ObjectCreatorBase
    {
    public:
        virtual ~ObjectCreatorBase() = default;
        virtual GameObject *CreateGameObject() = 0;
    };

    //  游戏对象构造器子类
    template <typename T>
    class ObjectCreator : public ObjectCreatorBase
    {
    public:
        virtual GameObject *CreateGameObject() override
        {
            return new T();
        }
    };

    //  游戏对象创建工厂
    class GameObjectFactory
    {
    public:
        //  获取游戏对象工厂实例
        static GameObjectFactory &GetInstance();

        template <typename T>
        void RegisterObject(const std::string &name)
        {
            m_creators.emplace(name, std::make_unique<ObjectCreator<T>>());
        }

        //  调用对应类型游戏对象创建器
        GameObject *GameObjectCreate(const std::string &typeName);

    private:
        std::unordered_map<std::string, std::unique_ptr<ObjectCreatorBase>> m_creators;
    };

#define GAMEOBJECT(ObjectClass) \
public:                         \
    static void Register() { GL_engine::GameObjectFactory::GetInstance().RegisterObject<ObjectClass>(std::string(#ObjectClass)); }
}

#endif