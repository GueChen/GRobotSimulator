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

    Ptr_KUKA_IIWA_MODEL getLeftRobot()  const;
    Ptr_KUKA_IIWA_MODEL getRightRobot() const;
    
    /// Tick Functions
    void tickImpl(float delta_time) override;
private:
    void InitializeModel();

/// 数据域 Fields
private:
    KUKA_IIWA_MODEL*  _left = nullptr, *_right = nullptr;
    ROBOT_BODY_MODEL* _body = nullptr;

    static bool pbr_init_;
};

}


#endif // DUAL_ARM_PLATFORM_H
