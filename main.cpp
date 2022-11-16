#include "mainwindow.h"
#include "engineapp.h"
#include <QtWidgets/QApplication>
// Delete Later
#include "model/robot/dual_arm_platform.h"
#include "model/robot/kuka_iiwa_model.h"
#include "manager/objectmanager.h"
#include "manager/modelmanager.h"
#include "component/collider_component.h"

#include <glm/glm.hpp>

void CreateCubeObstacle(float x, float y, float z, float x_l, float y_l, float z_l) {
    using namespace GComponent;
    static int idx = 0;
    static const std::string obj_name = "cube";
    ObjectManager::getInstance().CreateInstance(obj_name);
    Model* box = ModelManager::getInstance().GetModelByName(obj_name + std::to_string(idx++));
    box->setTransLocal(Vec3(x, y, z));
    box->setScale(Vec3(x_l, y_l, z_l));    
    box->RegisterComponent(std::make_unique<ColliderComponent>(box));
    auto col_com = box->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
    col_com->RegisterShape(new BoxShape(x_l * 0.5f, y_l * 0.5f, z_l * 0.5f));
}

int main(int argc, char *argv[])
{
    using namespace GComponent;
    GComponent::EngineApp::getInstance().Init(argc, argv);
    
    CreateCubeObstacle(0.5f,  0.8f,  1.0f,  0.2f,  0.7f,   0.3f);
    CreateCubeObstacle(-2.5f, -1.8f, 0.0f,  0.4f,  0.3f,   0.5f);
    //CreateCubeObstacle(0.35f, 0.4f,  0.2f,  0.1f,  0.25f,  0.1f);
    //CreateCubeObstacle(0.45f, 0.25f, 0.85f, 0.05f, 0.20f,  0.15f);
    //GComponent::KUKA_IIWA_MODEL   robot;
    //GComponent::DUAL_ARM_PLATFORM robot;
    GComponent::EngineApp::getInstance().Exec();
    
    return 0;
}
