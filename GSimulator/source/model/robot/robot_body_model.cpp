#include "robot_body_model.h"

#include "function/adapter/modelloader_qgladapter.h"
#include "manager/resourcemanager.h"
#include "manager/modelmanager.h"
#include "model/robot/robot_asset.h"
#include "model/robot/robot_model_builder.h"

#include "component/material_component.h"
#include "component/transform_component.h"

using namespace GComponent;

bool    ROBOT_BODY_MODEL::is_init_  = false;
size_t  ROBOT_BODY_MODEL::count_    = 0;
bool    ROBOT_BODY_MODEL::pbr_init_ = false;

ROBOT_BODY_MODEL::ROBOT_BODY_MODEL(Mat4 transform)
{    
    name_      = "dual_robot_body_" + std::to_string(count_++);
    mesh_      = "dual_arm_body";   
    GetTransform()->SetModelLocal(transform);    
    InitializeModelResource();
    ModelManager::getInstance().RegisteredModel(name_, this);

    RegisterComponent(std::make_unique<MaterialComponent>(this, "pbr", true));

   
}

void ROBOT_BODY_MODEL::InitializeModelResource()
{
    if(is_init_) return;
    const auto& platform_asset = GetDualArmPlatformAsset();
    ResourceManager::getInstance().RegisteredMesh(
        platform_asset.body_mesh.mesh_name,
        QGL::ModelLoader::getMeshPtr(platform_asset.body_mesh.mesh_path));
    is_init_ = true;
}

void GComponent::ROBOT_BODY_MODEL::tickImpl(float delta_time)
{
    if (!pbr_init_) {
        // Setting PBR Material Properties
        RobotModelBuilder::ApplyMaterialPreset(*this, GetDualArmPlatformAsset().body_material);
        pbr_init_ = true;
    }
}



