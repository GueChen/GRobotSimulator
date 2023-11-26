#include "model/robot/pgs_bot_platform.h"
#include "model/robot/aubo_i3_model.h"

#include "function/adapter/modelloader_qgladapter.h"
#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"
#include "manager/modelmanager.h"
#include "render/rendermesh.h"

#include "component/material_component.h"
#include "component/transform_component.h"

namespace GComponent{

static bool pgs_init = false;

GComponent::PGS_BOT_PLATFORM::PGS_BOT_PLATFORM(Mat4 transform)
{
    aubo_i3_robot_l_ = new AUBO_I3_MODEL(nullptr);
    aubo_i3_robot_r_ = new AUBO_I3_MODEL(nullptr);

    name_ = "pgs_bot";
    mesh_ = "aubo_i3_base";

    GetTransform()->SetModelLocal(transform);
    RegisterComponent(std::make_unique<MaterialComponent>(this, "pbr", true));

    InitializeModelResource();

    ModelManager::getInstance().RegisteredModel(name_, this);

    InitializeModel();
}

void GComponent::PGS_BOT_PLATFORM::InitializeModelResource()
{
    if (!pgs_init) {
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

        ResourceManager::getInstance().RegisteredMesh(mesh_, QGL::ModelLoader::getMeshPtr("./asset/stls/aubo_i3/aubo_i3_base.STL"));
        pgs_init = true;
    }
}

void GComponent::PGS_BOT_PLATFORM::InitializeModel()
{
    Eigen::Affine3f related_mat_l;
    related_mat_l.setIdentity();
    related_mat_l.translate(Vec3(0.0f, 0.204f, -0.21f));
    related_mat_l.rotate(Eigen::AngleAxisf(DegreeToRadius(-45.0f), Vec3::UnitX()));
    related_mat_l.rotate(Eigen::AngleAxisf(DegreeToRadius(0.0f), Vec3::UnitZ()));
    

    Eigen::Affine3f related_mat_r;
    related_mat_r.setIdentity();
    related_mat_r.translate(Vec3(0.0f, -0.204f, -0.21f));
    related_mat_r.rotate(Eigen::AngleAxisf(DegreeToRadius(45.0f), Vec3::UnitX()));
    related_mat_r.rotate(Eigen::AngleAxisf(DegreeToRadius(-180.0f), Vec3::UnitZ()));
    appendChild(aubo_i3_robot_l_, related_mat_l.matrix());
    appendChild(aubo_i3_robot_r_, related_mat_r.matrix());

    ModelManager::getInstance().ChangeModelParent(aubo_i3_robot_l_->getName(), getName());
    ModelManager::getInstance().ChangeModelParent(aubo_i3_robot_r_->getName(), getName());
}

}