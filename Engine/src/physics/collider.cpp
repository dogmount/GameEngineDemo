#include "physics/collider.h"

#include <btBulletCollisionCommon.h>

namespace GL_engine
{
    Collider::~Collider()
    {
        if (m_shape)
        {
            delete m_shape;
            m_shape = nullptr;
        }

        if (m_triangleMesh)
        {
            delete m_triangleMesh;
            m_triangleMesh = nullptr;
        }
    }

    //  获取碰撞形状
    btCollisionShape *Collider::GetShape()
    {
        return m_shape;
    }

    //  盒碰撞体
    BoxCollider::BoxCollider(const glm::vec3 &extents)
    {
        glm::vec3 halfExtents = extents * 0.5f;
        m_shape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
    }

    //  球碰撞体
    SphereCollider::SphereCollider(float radius)
    {
        m_shape = new btSphereShape(radius);
    }

    //  胶囊碰撞体
    CapsuleCollider::CapsuleCollider(float radius, float height)
    {
        m_shape = new btCapsuleShape(btScalar(radius), btScalar(height));
    }

    //  a/b/c: 底面三角形(局部坐标)   extrude: 挤出向量(方向+长度, 即棱柱的"棱")
    PrismCollider::PrismCollider(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec3 &extrude)
    {
        auto *hull = new btConvexHullShape();

        //  6 个角点 = 底面三点 + 各自沿挤出向量平移后的顶点
        //  意图: addPoint 第二参 false = 不逐点重算 AABB, 全部加完统一算一次
        const glm::vec3 pts[6] = {a, b, c, a + extrude, b + extrude, c + extrude};
        for (const auto &p : pts)
        {
            hull->addPoint(btVector3(p.x, p.y, p.z), false);
        }
        hull->recalcLocalAabb();

        m_shape = hull;
    }
}