

#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <vector>
namespace GComponent{

using glm::vec3;
using glm::mat4;
using glm::radians;
using glm::normalize;
using glm::cross;
using glm::dot;

enum class CameraMoveMent{
    FOWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// 默认值设置 Default Settings
constexpr float YAW         =  -90.0f;
constexpr float PITCH       =  0.0f;
constexpr float SPEED       =  2.5f;
constexpr float SENSITIVITY =  0.1f;
constexpr float ZOOM        =  45.f;

class Camera{
public:
    /// 数据域 Fields
    // 相机属性 Camera Attributes
    vec3 Position;                          // 相机位置 Camera Position
    vec3 Front;                             // 相机前向 Front Vector
    vec3 Up;                                // 相机上向 Up Vector
    vec3 Right;                             // 相机右向 Right Vector
    vec3 WorldUp;                           // 世界坐标上 World Up Vector
    // 欧拉角 Euler Angles
    float Yaw;
    float Pitch;
    // 相机选项 Camera Options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    /// 构造函数 Constructors
    Camera(vec3 position = vec3(0.0f, 0.5f, 8.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH):
        Front(glm::vec3(0.0f, 0.0f, -1.0f)),
            MovementSpeed(SPEED),
            MouseSensitivity(SENSITIVITY),
            Zoom(ZOOM)
        {
            Position = position;
            WorldUp = up;
            Yaw = yaw;
            Pitch = pitch;
            updateCameraVectors();
        }
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) :
        Front(glm::vec3(0.0f, 0.0f, -1.0f)),
            MovementSpeed(SPEED),
            MouseSensitivity(SENSITIVITY),
            Zoom(ZOOM)
        {
            Position = glm::vec3(posX, posY, posZ);
            WorldUp = glm::vec3(upX, upY, upZ);
            Yaw = yaw;
            Pitch = pitch;
            updateCameraVectors();
        }

    /// 析构函数 Destructors
    ~Camera() {};

    /// 成员函数 Methods
public:
    mat4 GetViewMatrix()
    {
        return MyLookAt(Position, Position + Front, Up);

    }
    void Move(float x, float y, float z)
    {
        Position += Front * z;
        Position += Right * x;
        Position += Up * y;
    }

    void Rotation(float yaw, float pitch, bool constrainPitch = true)
    {
        vec3 oriPos = vec3(0.0f, 0.2f, 0.0f);
        float length = glm::length(Position - oriPos);
        Yaw += yaw;
        Pitch += pitch;
        if(constrainPitch)
        {
            if(Pitch > 89.0f)
            {
                Pitch = 89.0f;
            }
            if(Pitch < -89.0f)
            {
                Pitch = -89.0f;
            }
        }
        updateCameraVectors();
        Position = oriPos - Front * length;
    }
private:
    /**
     * 更新相机坐标系的三个轴线方向
     * */
    void updateCameraVectors()
    {
        vec3 front;
        front.x = cos(radians(Yaw)) * cos(radians(Pitch));
        front.y = sin(radians(Pitch));
        front.z = sin(radians(Yaw)) * cos(radians(Pitch));

        Front = normalize(front);
        Right = normalize(cross(Front, WorldUp));
        Up = normalize(cross(Right, Front));
    }
    /**
     * 构造我的 LookAt 矩阵
     * @param pos {vec3} 当前相机位置
     * @param target {vec3} 视线看向的方向
     * @param up {vec3} 上向量方向
     * */
    mat4 MyLookAt(vec3 pos, vec3 target, vec3 up)
    {
        mat4 Result = mat4(1.0f);

        vec3 direction = normalize(pos - target);
        vec3 right     = normalize(cross(up, direction));
        vec3 cameraUp  = cross(direction, right);

        Result[0][0] = right.x;
        Result[1][0] = right.y;
        Result[2][0] = right.z;
        Result[0][1] = cameraUp.x;
        Result[1][1] = cameraUp.y;
        Result[2][1] = cameraUp.z;
        Result[0][2] = direction.x;
        Result[1][2] = direction.y;
        Result[2][2] = direction.z;
        Result[3][0] = -dot(right, target);
        Result[3][1] = -dot(cameraUp, target);
        Result[3][2] = -dot(direction, target);

        return Result;
    }
};

}
#endif // CAMERA_HPP
