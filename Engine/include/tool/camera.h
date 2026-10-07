#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//  枚举--定义可能的摄像机动作
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

//  定义摄像机数值
//  偏航角（Yaw）的默认值被设置为 - 90.0 度，其核心目的是让相机初始的朝向（Front 向量）指向世界坐标系的 - Z 轴方向
const float YAW = -90.f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f; // 鼠标移动速度系数
const float ZOOM = 45.0f;       //  fov数值

class Camera
{
public:
    // 摄像机属性
    glm::vec3 Position; //  位置
    glm::vec3 Front;    //  前向量
    glm::vec3 Up;       //  上向量
    glm::vec3 Right;    //  右向量
    glm::vec3 WorldUp;  //  世界上向量（用于求Up和Right）

    //  欧拉角
    float Yaw;
    float Pitch;
    //  摄像机选项
    float MovementSpeed;    //  摄像机移动速度
    float MouseSensitivity; //  鼠标移动速度系数
    float Zoom;             //  死区

    //  带向量的构造函数
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    //  带标量的构造函数
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    //  返回通过欧拉角和LookAt计算的view矩阵
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    //  处理从任何类似键盘的输入系统中接收的输入。接受以相机定义的ENUM形式输入参数(从窗口系统进行抽象)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += Up * velocity;
        if (direction == DOWN)
            Position -= Up * velocity;
    }

    // 处理从鼠标输入系统接收的输入。在 x 和 y 方向上均期望偏移值。
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // 限制俯仰角，避免万向节死锁
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // 更新摄像机向量
        updateCameraVectors();
    }

    //  通过鼠标滚轮设置缩放（fov）
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    //  通过（更新的）摄像机欧拉角，计算摄像机的各个向量
    void updateCameraVectors()
    {
        //  计算新的前向向量
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        //  计算右向量
        Right = glm::normalize(glm::cross(Front, WorldUp)); // 将向量标准化,因为其长度越接近0,向上或向下看越快,导致移动速度变慢
        //  计算上向量
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif