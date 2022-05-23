#include "robot_body_model.h"

#include "function/adapter/modelloader_qgladapter.h"
#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"
#include "manager/modelmanager.h"
#include "render/rendermesh.h"
#include "render/mygl.hpp"
#include "render/myshader.h"


using namespace GComponent;

bool    ROBOT_BODY_MODEL::is_init_  = false;
size_t  ROBOT_BODY_MODEL::count_    = 0;

ROBOT_BODY_MODEL::ROBOT_BODY_MODEL(mat4 transform)
{    
    name_      = "dual_robot_body_" + std::to_string(count_++);
    mesh_      = "dual_arm_body";
    shader_    = "color";
    setModelMatrix(transform);    
    InitializeResource();
    ModelManager::getInstance().RegisteredModel(name_, this);
}

void ROBOT_BODY_MODEL::InitializeResource()
{
    if(is_init_) return;
    ResourceManager::getInstance().RegisteredMesh(mesh_, new RenderMesh(QGL::ModelLoader::getMesh(sPathModel(string("body.STL")))));    
    is_init_ = true;
}

void GComponent::ROBOT_BODY_MODEL::tick(float delta_time)
{
    RenderManager::getInstance().EmplaceRenderCommand(name_, shader_, mesh_);
}

void GComponent::ROBOT_BODY_MODEL::setShaderProperty(MyShader& shader)
{
    shader.setMat4("model", getModelMatrix());
    shader.setVec3("color", color_);
    shader.setBool("NormReverse", false);
}

void ROBOT_BODY_MODEL::Draw(MyShader *shader)
{
    shader->setMat4("model", getModelMatrix());
    ResourceManager::getInstance().GetMeshByName(mesh_)->Draw();    
    for(auto & _child : children_)
    {
        _child->Draw(shader);
    }
}


