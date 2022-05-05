#include "dual_arm_platform.h"

#include "render/mygl.hpp"
#include "model/robot/kuka_iiwa_model.h"
#include "model/robot/robot_body_model.h"

using std::make_unique;
using std::make_shared;
using namespace GComponent;


void DUAL_ARM_PLATFORM::setGL(const shared_ptr<MyGL> &other)
{
    ROBOT_BODY_MODEL::setGL(other);
}

DUAL_ARM_PLATFORM::DUAL_ARM_PLATFORM(mat4 transform):
    _left(make_shared<KUKA_IIWA_MODEL>()),
    _right(make_shared<KUKA_IIWA_MODEL>()),
    _body(make_shared<ROBOT_BODY_MODEL>())
{
    model_mat_ = transform;
    InitializeModel();
}

void DUAL_ARM_PLATFORM::InitializeModel()
{
    mat4 im(1.0f);
    im = glm::translate(im, vec3(0.193f, 1.217f, 0.0f));
    im = glm::rotate(im, glm::radians(-30.0f), vec3(0.0f, 0.0f, 1.0f));
    im = glm::rotate(im, glm::radians(135.0f), vec3(0.0f, 1.0f, 0.0f));
    _body->appendChild(_left.get(), im);

    im = mat4(1.0f);
    im = glm::translate(im, vec3(-0.193f, 1.217f, 0.0f));
    im = glm::rotate(im, glm::radians(30.0f), vec3(0.0f, 0.0f, 1.0f));
    im = glm::rotate(im, glm::radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
    _body->appendChild(_right.get(), im);
}

void DUAL_ARM_PLATFORM::Draw(MyShader * shader)
{
    _body->Draw(shader);
}

void DUAL_ARM_PLATFORM::setLeftColor(const vec3 &color)
{
    _left->setColor(color);
}

void DUAL_ARM_PLATFORM::setRightColor(const vec3 &color)
{
    _right->setColor(color);
}

JointsPair DUAL_ARM_PLATFORM::getJoints() const
{
    array<Joint*, 7> left, right;
    for(int i = 0; i < 7; ++i)
    {
        left[i]  = reinterpret_cast<Joint*>(_left->Joints[i]);
        right[i] = reinterpret_cast<Joint*>(_right->Joints[i]);
    }
    return std::make_pair(left,right);
}

Ptr_KUKA_IIWA_MODEL DUAL_ARM_PLATFORM::getLeftRobot() const
{
    return _left.get();
}

Ptr_KUKA_IIWA_MODEL DUAL_ARM_PLATFORM::getRightRobot() const
{
    return _right.get();
}
