#include "kuka_iiwa_model.h"

#include "manager/modelmanager.h"
#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"

#include "render/rendermesh.h"

#include "component/joint_component.h"
#include "component/joint_group_component.h"
#include "component/kinematic_component.h"
#include "component/tracker_component.h"
#include "component/rigidbody_component.h"
#include "component/material_component.h"

#include "function/adapter/modelloader_qgladapter.h"

#include <stack>

#define IIWASource(name) "iiwa14_"#name".STL"

using namespace GComponent;
using std::make_unique;

bool KUKA_IIWA_MODEL::is_init_ = false;
int KUKA_IIWA_MODEL::count     = 0;

KUKA_IIWA_MODEL::KUKA_IIWA_MODEL(Mat4 transform) 
{    
    setModelMatrix(transform); 
    InitializeMeshResource();
    InitializeModelResource();
    ++count;
}

void KUKA_IIWA_MODEL::InitializeModelResource()
{            
    ModelManager&       model_manager = ModelManager::getInstance();
    array<Model*, 8>    models_tmp    = {nullptr};
    array<Vec3, 8>      local_trans   =
    {
//local transx        y         z           (.m)
        Vec3(0.0f,    0.0f,     0.0f),
        Vec3(0.0f,    0.0f,     0.1575f),
        Vec3(0.0f,    0.0f,     0.2025f),
        Vec3(0.0f,    0.0f,     0.2045f),
        Vec3(0.0f,    0.0f,     0.2155f),
        Vec3(0.0f,    0.0f,     0.1845f),
        Vec3(0.0f,    -0.0607f, 0.2155f),
        Vec3(0.0f,    0.0607f,  0.0809f)
    };

    // HARD-CODE PART    
    // register all links as model
    string count_str = "_" + std::to_string(count);
    model_manager.RegisteredModel("kuka_iiwa_robot" + count_str, this);
    for(int i = 0; i < 8; ++i)
    {
        string count_name = "kuka_iiwa_robot_link_" + std::to_string(i);
        models_tmp[i]     = new Model(count_name + count_str, 
                                      count_name, 
                                      local_trans[i], 
                                      Vec3::Zero(), 
                                      Vec3::Ones(),
                                      i > 0 ? models_tmp[i - 1] :this);
        models_tmp[i]->RegisterComponent(std::make_unique<MaterialComponent>(models_tmp[i], "pbr", true));
        model_manager.RegisteredModel(models_tmp[i]->getName(), models_tmp[i]);
    }
     
    // register joint component
    vector<Vec3> axis_binds = { Vec3::UnitZ(), Vec3::UnitY(),Vec3::UnitZ(), -Vec3::UnitY(), Vec3::UnitZ(), Vec3::UnitY(), Vec3::UnitZ() };
    for (int i = 1; i < 8; ++i) {
        models_tmp[i]->RegisterComponent(make_unique<JointComponent>(models_tmp[i], axis_binds[i - 1]));        
    } 
    constexpr const float kToler           = 3.5f;
    constexpr const float kLimitTable[][2] = {
        {DegreeToRadius(-170.0f + kToler), DegreeToRadius(180.0f - kToler)},
        {DegreeToRadius(-120.0f + kToler), DegreeToRadius(120.0f - kToler)},
        {DegreeToRadius(-170.0f + kToler), DegreeToRadius(170.0f - kToler)},
        {DegreeToRadius(-120.0f + kToler), DegreeToRadius(120.0f - kToler)},
        {DegreeToRadius(-170.0f + kToler), DegreeToRadius(170.0f - kToler)},
        {DegreeToRadius(-120.0f + kToler), DegreeToRadius(120.0f - kToler)},
        {DegreeToRadius(-175.0f + kToler), DegreeToRadius(175.0f - kToler)},
    };
    
    // setting joint limitations
    vector<JointComponent*> joints;
    for (int i = 1; i < 8; ++i) {
        JointComponent* joint = models_tmp[i]->GetComponent<JointComponent>("JointComponent");
        joint->SetPosLimit(kLimitTable[i - 1][0], kLimitTable[i - 1][1]);
        joints.push_back(joint);
    }

    // setting end effector poses
    SE3<float> init_end_trans_mat        = SE3<float>::Identity();
    init_end_trans_mat.block(0, 3, 3, 1) = Vec3(0.0f, 0.0f, 1.332f);
    
    // register rest component
    RegisterComponent(make_unique<JointGroupComponent>(this, joints));
    RegisterComponent(make_unique<KinematicComponent>(init_end_trans_mat, this));
    RegisterComponent(make_unique<TrackerComponent>(this));

    // register rigidbody component        
    using Aff = Eigen::Affine3f;
    CollisionGroup m_group{ 0, 0, 0, 0 };
    {// collider on base    
    Vec3  trans           = 0.5f * local_trans[1];
    Aff   local_transform = Aff::Identity();
    float half_length     = trans.norm();
    local_transform.translate(trans);
    models_tmp[0]->RegisterComponent(make_unique<RigidbodyComponent>(
                                            models_tmp[0], local_transform.matrix(), 
                                            half_length, half_length, half_length, 
                                            m_group));
    }

    const float kRadius   = 0.07f; 
    auto        mask_prod = [](const Vec3& input, const Vec3& mask)->Vec3 {
        return Vec3(input.x() * mask.x(), input.y() * mask.y(), input.z() * mask.z());
    };
    auto register_cylinder_collider_impl = [kRadius, m_group]
    (Model* model, Vec3 trans, float half_length, float radius) {
        Aff   local_transform = Eigen::Affine3f::Identity();
        local_transform.translate(trans).rotate(Eigen::AngleAxisf(EIGEN_PI * 0.5f, Vec3::UnitY()));
        model->RegisterComponent(make_unique<RigidbodyComponent>(
                                            model, local_transform.matrix(),
                                            radius, half_length,
                                            m_group));
    };
    auto register_cylinder_collider = [kRadius, &register_cylinder_collider_impl]
    (Model* model, Vec3 trans) {
        float half_length = trans.norm() - 0.07f;        
        register_cylinder_collider_impl(model, trans, half_length, /*0.07f)*/0.12f);
    };
    std::array<Vec3, 4> arm_trans = {
        0.5f * local_trans[2],
        0.5f * (local_trans[3] + local_trans[4]),
        0.5f * (local_trans[5] + mask_prod(local_trans[6], Vec3::UnitZ())),
        0.5f * local_trans[7]
    };
    // base extend
    register_cylinder_collider(models_tmp[1], 0.5f * local_trans[2]);
    //forearm
    //register_cylinder_collider(models_tmp[2], 0.5f * (local_trans[3] + local_trans[4]));
    // hin
    /*register_cylinder_collider_impl(
                                models_tmp[4], 0.4f * (local_trans[5] + local_trans[6]),
                                0.20f * (local_trans[5] + local_trans[6]).norm(),
                                kRadius + 0.08f);*/
    register_cylinder_collider_impl(
                               models_tmp[6], 0.45f * local_trans[7], 
                               0.35f * mask_prod(local_trans[7], Vec3::UnitZ()).norm(),
                               0.06f + 0.08f);    
}

void GComponent::KUKA_IIWA_MODEL::InitializeMeshResource()
{
    if (!is_init_)
    {
        ResourceManager& scene_manager = ResourceManager::getInstance();
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_0", QGL::ModelLoader::getMeshPtr(cPathModel("binary/iiwa14_base_binary.STL")));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_1", QGL::ModelLoader::getMeshPtr(cPathModel("binary/iiwa14_link_1_binary.STL")));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_2", QGL::ModelLoader::getMeshPtr(cPathModel("binary/iiwa14_link_2_binary.STL")));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_3", QGL::ModelLoader::getMeshPtr(cPathModel("binary/iiwa14_link_3_binary.STL")));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_4", QGL::ModelLoader::getMeshPtr(cPathModel("binary/iiwa14_link_4_binary.STL")));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_5", QGL::ModelLoader::getMeshPtr(cPathModel("binary/iiwa14_link_5_binary.STL")));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_6", QGL::ModelLoader::getMeshPtr(cPathModel("binary/iiwa14_link_6_binary.STL")));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_7", QGL::ModelLoader::getMeshPtr(cPathModel("binary/flansch_extern_binary.STL")));
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
    if (!pbr_init_) {
        std::stack<Model*> st;
        st.push(getChildren().front());
        while (!st.empty()) {
            auto cur = st.top(); st.pop();

            // Setting PBR Material Properties
            auto material = cur->GetComponent<MaterialComponent>(MaterialComponent::type_name.data());
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
                    val = 0.98f;
                }
                else if (name == "roughness") {
                    val = 0.25f;
                }
                else if (name == "albedo") {
                    val = Conversion::fromVec3f(_color);
                }
            }

            for (auto& child : cur->getChildren()) {
                st.push(child);
            }
        }
        pbr_init_ = true;
    }
}

void KUKA_IIWA_MODEL::setColor(const Vec3 &color)
{
    _color = color;
}

