#include "kuka_iiwa_model.h"

#include "model/robot/robot_asset.h"
#include "model/robot/robot_model_builder.h"

#include "component/transform_component.h"

#include <utility>

using namespace GComponent;

bool KUKA_IIWA_MODEL::is_init_ = false;
int KUKA_IIWA_MODEL::count     = 0;

KUKA_IIWA_MODEL::KUKA_IIWA_MODEL(Mat4 transform) 
{    
    GetTransform()->SetModelLocal(transform);    
    InitializeMeshResource();
    InitializeModelResource();
    ++count;
}

void KUKA_IIWA_MODEL::InitializeModelResource()
{
    auto built_model = RobotModelBuilder::Build(*this, GetKukaIiwaAsset(), "_" + std::to_string(count));
    SetRobotRuntimeData(std::move(built_model.links), std::move(built_model.joints));
}

void GComponent::KUKA_IIWA_MODEL::InitializeMeshResource()
{
    if (!is_init_)
    {
        RobotModelBuilder::RegisterMeshes(GetKukaIiwaAsset());
        is_init_ = true;
    }
}

void GComponent::KUKA_IIWA_MODEL::tickImpl(float delta_time)
{   
}

void KUKA_IIWA_MODEL::setColor(const Vec3 &color)
{
    _color = color;
}
