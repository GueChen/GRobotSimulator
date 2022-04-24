#ifndef DUAL_ARM_PLATFORM_H
#define DUAL_ARM_PLATFORM_H

#include <memory>

#include "model/model.h"

namespace GComponent {

class MyGL;
class Joint;
class KUKA_IIWA_MODEL;
class ROBOT_BODY_MODEL;

using std::unique_ptr;
using std::shared_ptr;
using std::array;
using std::pair;
using Ptr_KUKA_IIWA_MODEL = KUKA_IIWA_MODEL*;
using JointsPair = pair<array<Joint*, 7>, array<Joint*, 7>>;

class DUAL_ARM_PLATFORM: public Model
{
/// 成员函数 Member Function
public:
    explicit DUAL_ARM_PLATFORM(mat4 transform = mat4(1.0f));
    ~DUAL_ARM_PLATFORM() = default;
    void Draw(MyShader * shader) override;
    void setLeftColor(const vec3 & color);
    void setRightColor(const vec3 & color);

    JointsPair getJoints() const;
    Ptr_KUKA_IIWA_MODEL getLeftRobot() const;
    Ptr_KUKA_IIWA_MODEL getRightRobot() const;

    static void setGL(const shared_ptr<MyGL> & other);
private:
    void InitializeModel();

/// 数据域 Fields
private:
    shared_ptr<KUKA_IIWA_MODEL> _left, _right;
    shared_ptr<ROBOT_BODY_MODEL> _body;
};

}


#endif // DUAL_ARM_PLATFORM_H
