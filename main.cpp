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

#include <yvals.h>

int main(int argc, char *argv[])
{
    
    GComponent::EngineApp::getInstance().Init(argc, argv);
    
    GComponent::ObjectManager::getInstance().CreateInstance("sphere");
   
    {
        GComponent::Model* sphere_collider = GComponent::ModelManager::getInstance().GetModelByName("sphere0");
        if (sphere_collider) {
            sphere_collider->setTransLocal(Eigen::Vector3f(0.48f, 0.39f, 1.57f));
            sphere_collider->setScale(0.1f * Eigen::Vector3f::Ones());
            sphere_collider->RegisterComponent(std::make_unique<GComponent::RigidbodyComponent>(
                sphere_collider,
                Eigen::Matrix4f::Identity(),
                /*GComponent::ShapeEnum::Sphere,*/
                sphere_collider->getScale().x() * 0.5f,
                GComponent::CollisionGroup(1u, 0u, 0u, 0u)));
        }
    }

    GComponent::ObjectManager::getInstance().CreateInstance("cube");
    {
        GComponent::Model* cube_collider = GComponent::ModelManager::getInstance().GetModelByName("cube0");
        if (cube_collider) {
            cube_collider->setTransLocal(Eigen::Vector3f(0.48f, 0.25f, 1.57f));
            cube_collider->setScale(Eigen::Vector3f(0.20f, 0.02f, 0.20f));
            cube_collider->RegisterComponent(std::make_unique<GComponent::RigidbodyComponent>(
                cube_collider,
                Eigen::Matrix4f::Identity(),            
                cube_collider->getScale().x() * 0.5f,
                cube_collider->getScale().y() * 0.5f,
                cube_collider->getScale().z() * 0.5f,
                GComponent::CollisionGroup(1u, 0u, 0u, 0u)));
        }
    }

    GComponent::DUAL_ARM_PLATFORM robot;
    GComponent::EngineApp::getInstance().Exec();
    
    return 0;
}
