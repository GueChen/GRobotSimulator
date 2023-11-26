#include "model/robot/YDL_platform.h"
#include "model/robot/aubo_i5_model.h"

#include "function/adapter/modelloader_qgladapter.h"
#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"
#include "manager/modelmanager.h"
#include "render/rendermesh.h"

#include "component/material_component.h"
#include "component/transform_component.h"

static bool ydl_platform_is_init_ = false;

GComponent::YDLPlatform::YDLPlatform(Mat4 transform)
{
    aubo_i5_robot_ = new AUBO_I5_MODEL(nullptr);
    name_ = "ydl_shelf";
    mesh_ = "shelf";
    GetTransform()->SetModelLocal(transform);
    RegisterComponent(std::make_unique<MaterialComponent>(this, "pbr", true));
    
    InitializeModelResource();
    ModelManager::getInstance().RegisteredModel(name_, this);

    InitializeModel();
    
}

void GComponent::YDLPlatform::InitializeModelResource()
{
	if (!ydl_platform_is_init_) {
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

        ResourceManager::getInstance().RegisteredMesh(mesh_, QGL::ModelLoader::getMeshPtr("./asset/stls/aubo_i5/shelf.STL"));
		ydl_platform_is_init_ = true;
	}
}

void GComponent::YDLPlatform::InitializeModel()
{
    Eigen::Affine3f related_mat;
    related_mat.setIdentity();
    related_mat.translate(Vec3(0.18181f, 0.0f, 0.80983f));
    related_mat.rotate(Eigen::AngleAxisf(DegreeToRadius(45.0f), Vec3::UnitY()));

    appendChild(aubo_i5_robot_, related_mat.matrix());

    ModelManager::getInstance().ChangeModelParent(aubo_i5_robot_->getName(), getName());
}
