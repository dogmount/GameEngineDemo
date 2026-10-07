#include "scene/components/playerControlComponent.h"
#include "inputManager/inputManager.h"
#include "engine/Engine.h"
#include "scene/gameObject.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace GL_engine
{
    void PlayerControlComponent::Init()
    {
        m_kinematicControl = std::make_unique<KinematicCharacterControl>(0.4f, 1.2f, m_owner->GetWorldPosition());
    }

    void PlayerControlComponent::Update(float deltaTime)
    {
        auto &inputManager = Engine::GetInstance().GetInputManager();

        auto rotation = m_owner->GetRotation();

        if (inputManager.IsMousePositionChanged())
        {
            const auto &lastPos = inputManager.GetMousePositionLast();
            const auto &currentPos = inputManager.GetMousePositionCurrent();

            float deltaX = currentPos.x - lastPos.x;
            float deltaY = currentPos.y - lastPos.y;

            //  四元数旋转
            //  绕Y旋转
            // float yDeltaAngle = -deltaX * m_sensitivity * deltaTime;
            float yDeltaAngle = -deltaX * m_sensitivity; //  改进：不受帧率影响
            m_yRot += yDeltaAngle;
            glm::quat yRot = glm::angleAxis(glm::radians(m_yRot), glm::vec3(0.0f, 1.0f, 0.0f)); //  绕全局y轴旋转

            //  绕X旋转
            // float xDeltaAngle = -deltaY * m_sensitivity * deltaTime;
            float xDeltaAngle = -deltaY * m_sensitivity; //  改进：不受帧率影响
            m_xRot += xDeltaAngle;
            m_xRot = std::clamp(m_xRot, -89.0f, 89.0f);                                         //  限制俯仰角 -89.0f--89.0f
            glm::quat xRot = glm::angleAxis(glm::radians(m_xRot), glm::vec3(1.0f, 0.0f, 0.0f)); //  绕全局x轴旋转

            rotation = glm::normalize(yRot * xRot);

            m_owner->SetRotation(rotation);
        }

        /*
        旧移动向量计算：受到pitch角度影响，看向地板时无法移动，故此舍去

        glm::mat3 rotMat = glm::mat3_cast(rotation); // 将四元数转为 3x3 矩阵
        glm::vec3 right = rotMat[0];  //  局部 X 轴（右）
        glm::vec3 up = rotMat[1];     //  局部 Y 轴（上）
        glm::vec3 front = -rotMat[2]; //  局部 Z 轴，因为摄像机看向 -Z，所以取反
        */

        //  ---------- 移动基向量: 只用 yaw, 忽略 pitch ----------
        //  意图: 看向地板/天花板时仍能沿水平面正常移动, 且不存在"基向量退化"
        const glm::quat yawRotation = glm::angleAxis(glm::radians(m_yRot), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::vec3 front = yawRotation * glm::vec3(0.0f, 0.0f, -1.0f); //  水平前方
        const glm::vec3 right = yawRotation * glm::vec3(1.0f, 0.0f, 0.0f);  //  水平右方

        glm::vec3 move(0.0f);
        //  左右移动
        if (inputManager.IsKeyPressed(GLFW_KEY_A))
        {
            move -= right;
        }
        else if (inputManager.IsKeyPressed(GLFW_KEY_D))
        {
            move += right;
        }

        //  前后移动
        if (inputManager.IsKeyPressed(GLFW_KEY_S))
        {
            move -= front;
        }
        else if (inputManager.IsKeyPressed(GLFW_KEY_W))
        {
            move += front;
        }

        if (inputManager.IsKeyPressed(GLFW_KEY_SPACE))
        {
            m_kinematicControl->Jump(glm::vec3(0.0f, 5.0f, 0.0f));
        }

        //  归一化保证角色在任意方向上移动速度一致
        if (glm::dot(move, move) > 0)
        {
            move = glm::normalize(move);
        }
        const float physicsStep = Engine::GetInstance().GetPhysicsManager().GetFixedTimeStep();
        m_kinematicControl->Walk(move * m_moveSpeed * physicsStep);
        // m_kinematicControl->Walk(move * m_moveSpeed * deltaTime);//  原：由deltaTime驱动

        // //  只有"有意移动"时才推, 免得站着不动也把身边的箱子挤走
        // if (glm::dot(move, move) > 0.0f)
        // {
        //     m_kinematicControl->Push(2.5f); //  推挤速度 2.5 m/s, 按手感调
        // }

        m_owner->SetPosition(m_kinematicControl->GetPosition());
    }

    //  获取玩家与地面接触状态
    bool PlayerControlComponent::OnGround() const
    {
        if (m_kinematicControl)
        {
            return m_kinematicControl->OnGround();
        }
        return false;
    }
}