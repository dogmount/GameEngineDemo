#ifndef COLLIDER_H
#define COLLIDER_H
#pragma once

#include <glm/glm.hpp>

#include <vector>

class btCollisionShape;
class btTriangleMesh;

namespace GL_engine
{
    class Collider
    {
    public:
        virtual ~Collider();

        //  获取碰撞形状
        btCollisionShape *GetShape();

    protected:
        btCollisionShape *m_shape = nullptr;      //  碰撞形状
        btTriangleMesh *m_triangleMesh = nullptr; //  三棱柱碰撞体网格
    };

    class BoxCollider : public Collider
    {
    public:
        BoxCollider(const glm::vec3 &extents); //  盒碰撞体
    };

    class SphereCollider : public Collider
    {
    public:
        SphereCollider(float radius); //  球碰撞体
    };

    class CapsuleCollider : public Collider
    {
    public:
        CapsuleCollider(float radius, float height); //  胶囊碰撞体
    };

    class PrismCollider : public Collider
    {
    public:
        //  a/b/c: 底面三角形(局部坐标)   extrude: 挤出向量(方向+长度, 即棱柱的"棱")
        PrismCollider(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec3 &extrude); //  三棱柱碰撞体
    };
}

#endif