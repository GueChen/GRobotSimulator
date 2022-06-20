#include "kuka_iiwa_model.h"

#include "manager/modelmanager.h"
#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"
#include "render/myshader.h"
#include "render/rendermesh.h"
#include "component/joint_component.h"
#include "component/joint_group_component.h"
#include "component/kinematic_component.h"
#include "component/tracker_component.h"

#include "function/adapter/modelloader_qgladapter.h"

#define IIWASource(name) "iiwa14_"#name".STL"

using namespace GComponent;
using std::make_unique;

bool KUKA_IIWA_MODEL::is_init_ = false;
int KUKA_IIWA_MODEL::count     = 0;

KUKA_IIWA_MODEL::KUKA_IIWA_MODEL(Mat4 transform) 
{
    shader_ = "color";
    setModelMatrix(transform); 
    InitializeMeshResource();
    InitializeModelResource();
    ++count;
}

void KUKA_IIWA_MODEL::InitializeModelResource()
{            
    ModelManager&       model_manager = ModelManager::getInstance();
    array<Model*, 8>    models_tmp;
    array<Vec3, 8>      local_trans =
    {
        Vec3(0.0f, 0.0f, 0.0f),
        Vec3(0.0f, 0.0f, 0.1575f),
        Vec3(0.0f, 0.0f, 0.2025f),
        Vec3(0.0f, 0.0f, 0.2045f),
        Vec3(0.0f, 0.0f, 0.2155f),
        Vec3(0.0f, 0.0f, 0.1845f),
        Vec3(0.0f, -0.0607f, 0.2155f),
        Vec3(0.0f, 0.0607f, 0.0809f)
    };

    // HARD-CODE PART
    SE3<float> init_end_trans_mat        = SE3<float>::Identity();
    init_end_trans_mat.block(0, 3, 3, 1) = Vec3(0.0f, 0.0f, 1.332f);

    string count_str = "_" + std::to_string(count);
    model_manager.RegisteredModel("kuka_iiwa_robot" + count_str, this);
    for(int i = 0; i < 8; ++i)
    {
        string count_name = "kuka_iiwa_robot_link_" + std::to_string(i);
        models_tmp[i] = new Model(count_name + count_str, 
                                  count_name, 
                                  "color",
                                  local_trans[i], 
                                  Vec3::Zero(), 
                                  Vec3::Ones(),
                                  i > 0 ? models_tmp[i - 1] :this);
        model_manager.RegisteredModel(models_tmp[i]->getName(), models_tmp[i]);
    }
                
    vector<Vec3> axis_binds = { Vec3::UnitZ(), Vec3::UnitY(),Vec3::UnitZ(), -Vec3::UnitY(), Vec3::UnitZ(), Vec3::UnitY(), Vec3::UnitZ() };
    for (int i = 1; i < 8; ++i) {
        models_tmp[i]->RegisterComponent(make_unique<JointComponent>(models_tmp[i], axis_binds[i - 1]));
    }

    vector<JointComponent*> joints;
    for (int i = 1; i < 8; ++i) {
        joints.push_back(models_tmp[i]->GetComponet<JointComponent>("JointComponent"));
    }

    RegisterComponent(make_unique<JointGroupComponent>(joints, this));
    RegisterComponent(make_unique<KinematicComponent>(init_end_trans_mat, this));
    RegisterComponent(make_unique<TrackerComponent>(this));
}

void GComponent::KUKA_IIWA_MODEL::InitializeMeshResource()
{
    if (!is_init_)
    {
        ResourceManager& scene_manager = ResourceManager::getInstance();
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_0", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(base))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_1", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_1))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_2", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_2))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_3", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_3))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_4", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_4))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_5", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_5))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_6", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_6))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_7", QGL::ModelLoader::getMeshPtr(cPathModel("flanschExten.STL")));
        is_init_ = true;
    }
}

void GComponent::KUKA_IIWA_MODEL::setShaderProperty(MyShader & shader)
{
    shader.setMat4("model", Conversion::fromMat4f(getModelMatrix()));
    shader.setVec3("color", Conversion::fromVec3f(_color));
    shader.setBool("NormReverse", false);
}

void GComponent::KUKA_IIWA_MODEL::tickImpl(float delta_time)
{   
    RenderManager::getInstance().EmplaceRenderCommand(name_, shader_, mesh_);
}

void KUKA_IIWA_MODEL::setColor(const Vec3 &color)
{
    _color = color;
}

void KUKA_IIWA_MODEL::AddCheckPoint(int idx, const WeightedCheckPoint &p)
{
    _checkPointDict[idx].emplace_back(p);
}
