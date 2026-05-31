#include "dual_arm_platform.h"

#include "manager/modelmanager.h"
#include "model/robot/kuka_iiwa_model.h"
#include "model/robot/robot_body_model.h"
#include "model/robot/robot_asset.h"
#include "model/robot/robot_model_builder.h"
#include "component/tracker_component.h"

#include "component/kinematic_component.h"
#include "component/material_component.h"
#include "component/transform_component.h"

using std::make_unique;
using std::make_shared;
using namespace GComponent;

bool DUAL_ARM_PLATFORM::pbr_init_ = false;

DUAL_ARM_PLATFORM::DUAL_ARM_PLATFORM(Mat4 transform)
{
    _left  = new  KUKA_IIWA_MODEL;
    _right = new  KUKA_IIWA_MODEL;
    _body  = new  ROBOT_BODY_MODEL;
    GetTransform()->SetModelLocal(transform);
    InitializeModel();
}

void GComponent::DUAL_ARM_PLATFORM::tickImpl(float delta_time)
{}

void DUAL_ARM_PLATFORM::InitializeModel()
{
    const auto& platform_asset = GetDualArmPlatformAsset();

    _body->appendChild(_left,  platform_asset.left_arm.local_transform);
    _body->appendChild(_right, platform_asset.right_arm.local_transform);

    _left->setColor(Vec3(0.8f, 0.6f, 0.2f));
    _right->setColor(Vec3(0.2f, 0.6f, 0.8f));
    ModelManager::getInstance().ChangeModelParent(_left->getName(),  _body->getName());
    ModelManager::getInstance().ChangeModelParent(_right->getName(), _body->getName());

    RobotModelBuilder::ApplyMaterialPreset(*getLeftRobot(), platform_asset.left_arm.material);
    RobotModelBuilder::ApplyMaterialPreset(*getRightRobot(), platform_asset.right_arm.material);
}

Ptr_KUKA_IIWA_MODEL DUAL_ARM_PLATFORM::getLeftRobot() const
{
    return _left;
}

Ptr_KUKA_IIWA_MODEL DUAL_ARM_PLATFORM::getRightRobot() const
{
    return _right;
}
