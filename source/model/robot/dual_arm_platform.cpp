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


DUAL_ARM_PLATFORM::DUAL_ARM_PLATFORM(Mat4 transform)
{
    _left  = new  KUKA_IIWA_MODEL;
    _right = new  KUKA_IIWA_MODEL;
    _body  = new  ROBOT_BODY_MODEL;
    setModelMatrix(transform);
    InitializeModel();
}

void DUAL_ARM_PLATFORM::InitializeModel()
{
    Eigen::Affine3f left_model_mat, right_model_mat;
    
    left_model_mat.setIdentity();    
    left_model_mat.translate(Vec3(0.0f, 0.193f, 1.217f));
    left_model_mat.rotate(Eigen::AngleAxisf(DegreeToRadius(-30.0f), Vec3::UnitX()));
    left_model_mat.rotate(Eigen::AngleAxisf(DegreeToRadius(45.0f), Vec3::UnitZ()));
    
    right_model_mat.setIdentity();
    right_model_mat.translate(Vec3(0.0f, -0.193f, 1.217f));
    right_model_mat.rotate(Eigen::AngleAxisf(DegreeToRadius(30.0f), Vec3::UnitX()));
    right_model_mat.rotate(Eigen::AngleAxisf(DegreeToRadius(-45.0f), Vec3::UnitZ()));

    _body->appendChild(_left,  left_model_mat.matrix());
    _body->appendChild(_right, right_model_mat.matrix());

    _left->RegisterComponent(std::make_unique<TrackerComponent>(_left, GComponent::ModelManager::getInstance().GetModelByName("sphere0")));
            
    _left->setColor(vec3(0.8f, 0.6f, 0.2f));
    _right->setColor(vec3(0.2f, 0.6f, 0.8f));
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
