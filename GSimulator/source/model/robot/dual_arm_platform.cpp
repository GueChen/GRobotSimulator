#include "dual_arm_platform.h"

#include "manager/modelmanager.h"
#include "render/mygl.hpp"
#include "model/robot/kuka_iiwa_model.h"
#include "model/robot/robot_body_model.h"
#include "component/tracker_component.h"

#include "component/kinematic_component.h"
#include "component/material_component.h"
#include "component/transform_component.h"

#include <memory>
#include <stack>
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

    _left->setColor(Vec3(0.8f, 0.6f, 0.2f));
    _right->setColor(Vec3(0.2f, 0.6f, 0.8f));
    ModelManager::getInstance().ChangeModelParent(_left->getName(),  _body->getName());
    ModelManager::getInstance().ChangeModelParent(_right->getName(), _body->getName());



    // right arm
    {
        Model* left = getLeftRobot();
        std::stack<Model*> st; st.push(left->getChildren().front());
        while (!st.empty()) {
            Model* cur = st.top(); st.pop();
            // Setting PBR Material Properties
            auto material = cur->GetComponent<MaterialComponent>();
            auto& props = material->GetProperties();            
            for (auto& [_, name, __, val] : props) {
                if (name == "accept shadow") {
                    val = true;
                }
                else if (name == "albedo color") {
                    val = glm::vec3(0.80f, 0.05f, .00f);
                }
                else if (name == "ao") {
                    val = 0.05f;
                }
                else if (name == "metallic") {
                    val = 0.98f;
                }
                else if (name == "roughness") {
                    val = 0.25f;
                }
            }
            auto children = cur->getChildren();
            for (auto& child : children) { st.push(child); }
        }
    }
    {
        Model* right = getRightRobot();
        std::stack<Model*> st; st.push(right->getChildren().front());
        while (!st.empty()) {
            Model* cur = st.top(); st.pop();            
            // Setting PBR Material Properties
            auto material = cur->GetComponent<MaterialComponent>();
            auto& props = material->GetProperties();            
            for (auto& [_, name, __, val] : props) {
                if (name == "accept shadow") {
                    val = true;
                }
                else if (name == "albedo color") {
                    val = glm::vec3(1.00f, 1.00f, 1.00f);
                }
                else if (name == "ao") {
                    val = 0.05f;
                }
                else if (name == "metallic") {
                    val = 0.25f;
                }
                else if (name == "roughness") {
                    val = 0.25f;
                }
            }
            auto children = cur->getChildren();
            for (auto& child : children) { st.push(child); }
        }
    }
}

Ptr_KUKA_IIWA_MODEL DUAL_ARM_PLATFORM::getLeftRobot() const
{
    return _left;
}

Ptr_KUKA_IIWA_MODEL DUAL_ARM_PLATFORM::getRightRobot() const
{
    return _right;
}
