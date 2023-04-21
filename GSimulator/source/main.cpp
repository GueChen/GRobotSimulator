#include "ui/mainwindow.h"
#include "engineapp.h"

#include <QtWidgets/QApplication>
// TODO: add Serializer and Deserializer then Delete Later
#include "model/robot/dual_arm_platform.h"
#include "model/robot/kuka_iiwa_model.h"
#include "manager/objectmanager.h"
#include "manager/modelmanager.h"
#include "component/collider_component.h"

#include <glm/glm.hpp>
#include <map>

void CreateSphereObstacle(float x, float y, float z, float radius) {
    using namespace GComponent;
    static int idx = 0;
    static const std::string obj_name = "sphere";
    ObjectManager::getInstance().CreateInstance(obj_name);
    Model* sphere = ModelManager::getInstance().GetModelByName(obj_name + std::to_string(idx++));
    sphere->setTransLocal(Vec3(x, y, z));
    sphere->setScale(Vec3::Ones()* radius);
    sphere->RegisterComponent(std::make_unique<ColliderComponent>(sphere));
    auto col_com = sphere->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
    col_com->RegisterShape(new SphereShape(0.5f * radius));
}

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

void CreateCapsuleObstacle(float x, float y, float z, float radius, float half_z) {
    using namespace GComponent;
    static int idx = 0;
    static const std::string obj_name = "capsule";
    ObjectManager::getInstance().CreateInstance(obj_name);
    Model* capsule = ModelManager::getInstance().GetModelByName(obj_name + std::to_string(idx++));
    capsule->setTransLocal(Vec3(x, y, z));
    capsule->setScale(Vec3(radius / 0.3, radius / 0.3, (half_z/ 0.7 + radius / 0.3) * 0.5f));
    capsule->RegisterComponent(std::make_unique<ColliderComponent>(capsule));
    auto col_com = capsule->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
    col_com->RegisterShape(new CapsuleShape(radius, half_z));
}

int main(int argc, char *argv[])
{
    using namespace GComponent;
    GComponent::EngineApp::getInstance().Init(argc, argv);
    
   /* CreateCubeObstacle(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    CreateSphereObstacle(0.0f, 0.0f, 0.0f, 1.0f);*/
   /* CreateCubeObstacle(0.5f,    0.8f,  1.0f,  0.2f,  0.7f,   0.3f);
    CreateCubeObstacle(-2.5f,   -1.8f, 0.0f,  0.4f,  0.3f,   0.5f);
    CreateSphereObstacle(0.35f, 0.45f, 2.5f, 0.8f);
    
    CreateCubeObstacle(-0.30f, -0.4f, 1.2f, 0.5f, 0.5f, 0.5f);
    CreateCapsuleObstacle(0.55f, -0.5f, 0.0f, 0.1f, 0.3f);
    CreateCapsuleObstacle(0.0f, 0.0f, 0.0f, 0.3f, 0.7f);
    CreateSphereObstacle(0.0f, 0.0f, 0.0f, 1.0f);
    CreateCubeObstacle(0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);*/

    GComponent::DUAL_ARM_PLATFORM platform;

    GComponent::EngineApp::getInstance().Exec();
    
    return 0;
}
