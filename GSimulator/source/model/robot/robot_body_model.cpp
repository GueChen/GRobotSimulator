#include "robot_body_model.h"

#include "function/adapter/modelloader_qgladapter.h"
#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"
#include "manager/modelmanager.h"
#include "render/rendermesh.h"

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
    ResourceManager::getInstance().RegisteredMesh(mesh_, new RenderMesh(QGL::ModelLoader::getMesh(sPathModel(string("binary/platform_binary.STL")))));    
    is_init_ = true;
}

void GComponent::ROBOT_BODY_MODEL::tickImpl(float delta_time)
{
    if (!pbr_init_) {
        // Setting PBR Material Properties
        auto material = GetComponent<MaterialComponent>();
        auto& props = material->GetProperties();
        if (props.empty()) return;
        for (auto& [_, name, __, val] : props) {
            if (name == "accept shadow") {
                val = true;
            }
            else if (name == "ao") {
                val = 0.05f;
            }
            else if (name == "metallic") {
                val = 1.00f;
            }
            else if (name == "roughness") {
                val = 0.25f;
            }
        }
        pbr_init_ = true;
    }
}




