#include "mainwindow.h"
#include "engineapp.h"
#include <QtWidgets/QApplication>
// Delete Later
#include "model/robot/dual_arm_platform.h"
#include "manager/objectmanager.h"
#include "manager/modelmanager.h"
#include "component/tracker_component.h"
#include "component/rigidbody_component.h"
#include <glm/glm.hpp>

int main(int argc, char *argv[])
{
    
    GComponent::EngineApp::getInstance().Init(argc, argv);
    
    GComponent::ObjectManager::getInstance().CreateInstance("sphere");
    GComponent::Model* sphere_collider = GComponent::ModelManager::getInstance().GetModelByName("sphere0");
    if (sphere_collider) {
        sphere_collider->setTransLocal(Eigen::Vector3f(0.25f, 0.85f, 1.75f));
        sphere_collider->setScale(0.1f * Eigen::Vector3f::Ones());
        sphere_collider->RegisterComponent(std::make_unique<GComponent::RigidbodyComponent>(
            sphere_collider,
            Eigen::Matrix4f::Identity(),
            /*GComponent::ShapeEnum::Sphere,*/
            sphere_collider->getScale().x()));
    }

    GComponent::DUAL_ARM_PLATFORM robot;
    GComponent::EngineApp::getInstance().Exec();
    
    return 0;
}
