#include "dual_arm_platform.h"

#include "manager/modelmanager.h"
#include "render/mygl.hpp"
#include "model/robot/kuka_iiwa_model.h"
#include "model/robot/robot_body_model.h"
#include "component/tracker_component.h"

#include <memory>

using std::make_unique;
using std::make_shared;
using namespace GComponent;


DUAL_ARM_PLATFORM::DUAL_ARM_PLATFORM(mat4 transform)
{
    _left  = new  KUKA_IIWA_MODEL;
    _right = new  KUKA_IIWA_MODEL;
    _body  = new  ROBOT_BODY_MODEL;
    setModelMatrix(transform);
    InitializeModel();
}

void DUAL_ARM_PLATFORM::InitializeModel()
{
    mat4 im(1.0f);
    im = glm::translate(im, vec3(0.0, 0.193f, 1.217f));
    im = glm::rotate(im, glm::radians(-30.0f), vec3(1.0f, 0.0f, 0.0f));
    im = glm::rotate(im, glm::radians(45.0f), vec3(0.0f, 0.0f, 1.0f));
    _body->appendChild(_left, im);
    //_left->RegisterComponent(std::make_unique<TrackerComponent>(_left, GComponent::ModelManager::getInstance().GetModelByName("sphere0")));
    im = mat4(1.0f);
    im = glm::translate(im, vec3(0.0f, -0.193f, 1.217f));
    im = glm::rotate(im, glm::radians(30.0f), vec3(1.0f, 0.0f, 0.0f));
    im = glm::rotate(im, glm::radians(-45.0f), vec3(0.0f, 0.0f, 1.0f));
    _body->appendChild(_right, im);

    ModelManager::getInstance().ChangeModelParent(_left->getName(),  _body->getName());
    ModelManager::getInstance().ChangeModelParent(_right->getName(), _body->getName());
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

Ptr_KUKA_IIWA_MODEL DUAL_ARM_PLATFORM::getLeftRobot() const
{
    return _left;
}

Ptr_KUKA_IIWA_MODEL DUAL_ARM_PLATFORM::getRightRobot() const
{
    return _right;
}
