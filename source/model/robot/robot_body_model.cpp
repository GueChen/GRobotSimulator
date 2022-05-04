#include "robot_body_model.h"
#include "function/modelloader.h"

#include "component/mesh_component.h"
#include "render/mygl.hpp"
#include "render/myshader.h"


using namespace GComponent;

bool             ROBOT_BODY_MODEL::hasInit  = false;
unique_ptr<MeshComponent> ROBOT_BODY_MODEL::Resource = nullptr;

void ROBOT_BODY_MODEL::setGL(const shared_ptr<MyGL> &other)
{
    Resource->setGL(other);
}

ROBOT_BODY_MODEL::ROBOT_BODY_MODEL(mat4 transform)
{
    _matrixModel = transform;
    InitializeResource();
}

void ROBOT_BODY_MODEL::InitializeResource()
{
    if(hasInit) return;
    Resource = std::make_unique<MeshComponent>(
                ModelLoader::getMesh(sPathModel(string("body.STL"))));
    hasInit = true;
}

void ROBOT_BODY_MODEL::Draw(MyShader *shader)
{
    shader->setMat4("model", getModelMatrix());
    Resource->Draw();
    for(auto & [_child, _] : _children)
    {
        _child->Draw(shader);
    }
}
