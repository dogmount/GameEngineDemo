#ifndef OBJECT_COMPONENT_H
#define OBJECT_COMPONENT_H
#pragma once

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>
#include <unordered_map>
#include <memory>

namespace GL_engine
{
    class GameObject;

    class Component
    {
    public:
        virtual ~Component() = default;
        virtual void LoadProperties(const nlohmann::json &json);
        virtual void Init();
        virtual void Update(float deltaTime);

        //  获取类型id
        virtual size_t GetTypeId() const = 0;

        //  获取组件所有者指针
        GameObject *GetOwner();

        //  模板设置组件类型id
        template <typename T>
        static size_t StaticTypeId()
        {
            static size_t typeId = nextID++;
            return typeId;
        }

    protected:
        friend class GameObject;
        GameObject *m_owner = nullptr; //  组件所有者

    private:
        static size_t nextID; //  组件id计数器
    };

    //  模板基类（组件构造器）---类型擦除，使得ComponentFactory中unoedered_map可以存储各种组件构造器
    class ComponentCreatorBase
    {
    public:
        virtual ~ComponentCreatorBase() = default;
        virtual Component *CreateComponent() = 0;
    };

    //  模板子类（组件构造器）
    template <typename T>
    class ComponentCreator : public ComponentCreatorBase
    {
    public:
        Component *CreateComponent() override
        {
            return new T();
        }
    };

    //  组件工厂
    class ComponentFactory
    {
    public:
        static ComponentFactory &GetInstance();

        //  模板登记组件构造器
        template <typename T>
        void RegisterComponent(const std::string &name)
        {
            m_creators.emplace(name, std::make_unique<ComponentCreator<T>>());
            m_parentMap[T::TypeId()].push_back(Component::StaticTypeId<Component>());
        }

        template <typename T, typename ParentType>
        void RegisterComponent(const std::string &name)
        {
            m_creators.emplace(name, std::make_unique<ComponentCreator<T>>());
            m_parentMap[T::TypeId()].push_back(Component::StaticTypeId<ParentType>());
        }

        Component *ComponentCreate(const std::string &name);

        bool HasParent(size_t objectType, size_t parentType);

    private:
        std::unordered_map<std::string, std::unique_ptr<ComponentCreatorBase>> m_creators;

        std::unordered_map<size_t, std::vector<size_t>> m_parentMap; //  组件派生继承链（<组件类型ID，直接父集合>）
    };

#define COMPONENT(ComponentClass)                                                           \
public:                                                                                     \
    static size_t TypeId() { return GL_engine::Component::StaticTypeId<ComponentClass>(); } \
    size_t GetTypeId() const override { return TypeId(); }                                  \
    static void Register() { GL_engine::ComponentFactory::GetInstance().RegisterComponent<ComponentClass>(std::string(#ComponentClass)); }

#define COMPONENT_RELATE(ComponentClass, ParentComponentClass)                              \
public:                                                                                     \
    static size_t TypeId() { return GL_engine::Component::StaticTypeId<ComponentClass>(); } \
    size_t GetTypeId() const override { return TypeId(); }                                  \
    static void Register() { GL_engine::ComponentFactory::GetInstance().RegisterComponent<ComponentClass, ParentComponentClass>(std::string(#ComponentClass)); }
}

#endif