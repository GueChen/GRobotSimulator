#include "robot_body_model.h"
#include "Component/Geometry/modelloader.h"
#include "Component/Geometry/mesh.h"
#include "Component/myshader.h"
#include "Component/MyGL.hpp"

using namespace GComponent;

unique_ptr<Mesh> ROBOT_BODY_MODEL::Resource = nullptr;
bool ROBOT_BODY_MODEL::hasInit = false;

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
    Resource = std::make_unique<Mesh>(
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
