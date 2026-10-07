#include "scene/components/cameraComponent.h"
#include "scene/gameObject.h"
#include <glm/gtc/matrix_transform.hpp>

namespace GL_engine
{
    void CameraComponent::Update(float deltaTime)
    {
    }

    //  获取视图矩阵
    glm::mat4 CameraComponent::GetViewMatrix() const
    {
        glm::mat4 mat = glm::mat4(1.0f);
        mat = glm::mat4_cast(m_owner->GetRotation()); //  获取相机旋转矩阵

        mat[3] = glm::vec4(m_owner->GetPosition(), 1.0f); //  将位移向量合并进旋转矩阵（获得相机旋转+位移矩阵）

        if (m_owner->GetParent())
        {
            mat = m_owner->GetParent()->GetWorldTransform() * mat;
        }

        return glm::inverse(mat);
    }

    //  获取投影矩阵
    glm::mat4 CameraComponent::GetProjectionMatrix(float aspect) const
    {
        return glm::perspective(glm::radians(m_fov), aspect, m_nearPlane, m_farPlane);
    }

    //  获取相机位置
    glm::vec3 CameraComponent::GetEyePosition() const
    {
        glm::mat4 mat = glm::mat4(1.0f);
        mat = glm::mat4_cast(m_owner->GetRotation()); //  获取相机旋转矩阵

        mat[3] = glm::vec4(m_owner->GetPosition(), 1.0f); //  将位移向量合并进旋转矩阵（获得相机旋转+位移矩阵）

        if (m_owner->GetParent())
        {
            mat = m_owner->GetParent()->GetWorldTransform() * mat;
        }

        return glm::vec3(mat[3]); //  相机世界矩阵的平移列 = 眼睛位置
    }
}