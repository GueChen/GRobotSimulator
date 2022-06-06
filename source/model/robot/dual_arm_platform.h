#ifndef DUAL_ARM_PLATFORM_H
#define DUAL_ARM_PLATFORM_H

#include "model/model.h"

#include <memory>

#include <GComponent/GNumerical.hpp>

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
    explicit DUAL_ARM_PLATFORM(Mat4 transform = Mat4::Identity());
    ~DUAL_ARM_PLATFORM() = default;

    void Draw(MyShader * shader) override;
    
    void setLeftColor(const Vec3 & color);
    void setRightColor(const Vec3 & color);

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
