#ifndef DUAL_ARM_PLATFORM_H
#define DUAL_ARM_PLATFORM_H

#include <memory>

#include "model/model.h"

namespace GComponent {

class MyGL;
class KUKA_IIWA_MODEL;
class ROBOT_BODY_MODEL;

using std::array;
using std::pair;
using Ptr_KUKA_IIWA_MODEL = KUKA_IIWA_MODEL*;

class DUAL_ARM_PLATFORM: public Model
{
/// 成员函数 Member Function
public:
    explicit DUAL_ARM_PLATFORM(mat4 transform = mat4(1.0f));
    ~DUAL_ARM_PLATFORM() = default;

    void Draw(MyShader * shader) override;
    
    void setLeftColor(const vec3 & color);
    void setRightColor(const vec3 & color);

    Ptr_KUKA_IIWA_MODEL getLeftRobot()  const;
    Ptr_KUKA_IIWA_MODEL getRightRobot() const;

private:
    void InitializeModel();

/// 数据域 Fields
private:
    KUKA_IIWA_MODEL*  _left = nullptr, *_right = nullptr;
    ROBOT_BODY_MODEL* _body = nullptr;
};

}


#endif // DUAL_ARM_PLATFORM_H
