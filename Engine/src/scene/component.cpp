#include "scene/component.h"

namespace GL_engine
{
    size_t Component::nextID = 1;

    void Component::LoadProperties(const nlohmann::json &json)
    {
    }

    void Component::Init()
    {
    }

    void Component::Update(float deltaTime)
    {
    }

    //  获取组件所有者指针
    GameObject *Component::GetOwner()
    {
        return m_owner;
    }

    ComponentFactory &ComponentFactory::GetInstance()
    {
        static ComponentFactory instance;
        return instance;
    }

    Component *ComponentFactory::ComponentCreate(const std::string &name)
    {
        auto it = m_creators.find(name);
        if (it != m_creators.end())
        {
            return it->second->CreateComponent();
        }

        return nullptr;
    }

    bool ComponentFactory::HasParent(size_t objectType, size_t parentType)
    {
        auto record = m_parentMap.find(objectType);
        if (record == m_parentMap.end())
        {
            return false;
        }

        auto &parents = record->second;
        if (std::find(parents.begin(), parents.end(), parentType) != parents.end())
        {
            return true;
        }

        for (auto p : parents)
        {
            if (HasParent(p, parentType))
            {
                return true;
            }
        }
        return false;
    }
}