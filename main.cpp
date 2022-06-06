#include "mainwindow.h"
#include "engineapp.h"
#include <QtWidgets/QApplication>
// Delete Later
#include "model/robot/dual_arm_platform.h"
#include "manager/objectmanager.h"
#include "manager/modelmanager.h"
#include <glm/glm.hpp>

int main(int argc, char *argv[])
{
    
    GComponent::EngineApp::getInstance().Init(argc, argv);
    GComponent::ObjectManager::getInstance().CreateInstance("plane");
    //GComponent::ObjectManager::getInstance().CreateInstance("sphere");

    /*GComponent::Model * sphere = GComponent::ModelManager::getInstance().GetModelByName("sphere0");
    if (sphere) {
        sphere->setTransLocal(Eigen::Vector3f(0.0f, 1.3f, 2.0f));
        sphere->setScale(0.05f * Eigen::Vector3f::Ones());
    }*/
    GComponent::DUAL_ARM_PLATFORM robot;
    GComponent::EngineApp::getInstance().Exec();
    
    return 0;
}
