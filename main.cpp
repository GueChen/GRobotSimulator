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
    GComponent::ObjectManager::getInstance().CreateInstance("sphere");
    GComponent::Model * sphere = GComponent::ModelManager::getInstance().GetModelByName("sphere0");
    if (sphere) {
        sphere->setTransLocal(glm::vec3(0.0, 1.3, 2.0f));
        sphere->setScale(glm::vec3(0.05f));
    }
    GComponent::DUAL_ARM_PLATFORM robot;
    GComponent::EngineApp::getInstance().Exec();
    
    return 0;
}
