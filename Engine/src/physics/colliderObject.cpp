#include "physics/colliderObject.h"

#include <algorithm>

namespace GL_engine
{
    //  获取碰撞对象类型
    CollisionObjectType CollisionObject::GetCollisionObjectType()
    {
        return m_collisionObjectType;
    }

    //  添加/移除接触监听器
    void CollisionObject::AddContactListener(MyContactListener *listener)
    {
        m_contactListeners.push_back(listener);
    }
    void CollisionObject::RemoveContactListener(MyContactListener *listener)
    {
        auto it = std::find(m_contactListeners.begin(), m_contactListeners.end(), listener);
        if (it != m_contactListeners.end())
        {
            m_contactListeners.erase(it);
        }
    }

    //  分发接触事件
    void CollisionObject::DispatchContactEvent(
        CollisionObject *obj,
        const glm::vec3 &pos,
        const glm::vec3 &normal)
    {
        for (auto listener : m_contactListeners)
        {
            if (listener)
            {
                listener->OnContact(obj, pos, normal);
            }
        }
    }
}