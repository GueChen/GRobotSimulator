#include "model.h"

#include "manager/modelmanager.h"
#include "manager/rendermanager.h"

#include "render/myshader.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <format>

//#define _DEBUG

using namespace GComponent;

Model::Model(Model * parent, const string & meshKey):
    mesh_(meshKey), parent_(parent)
{}

Model::~Model() = default;

void Model::setModelMatrix(const mat4 &mat)
{
    model_mat_ = mat;
    updateChildrenMatrix();
}

void Model::setAxis(vec3 axis)
{
    axis_ = axis;
}

void Model::setRotate(float angle)
{
    mat4 mat = glm::identity<mat4>();
    model_mat_ = glm::rotate(mat, glm::radians(angle), axis_);
    updateChildrenMatrix();
}

void Model::appendChild(const _pModel &pchild, mat4 transform)
{
    pchild->setParent(this);
    children_.emplace_back(std::make_pair(pchild, transform));
    updateChildrenMatrix();
}

const vector<pair<_pModel, mat4>>& Model::getChildren() const
{
    return children_;
}

void Model::updateChildrenMatrix()
{
    for(auto & [child, transf]: children_)
    {
        child->parent_model_mat_ = parent_model_mat_ * model_mat_ * transf;
        child->updateChildrenMatrix();
    }
}

void GComponent::Model::setShaderProperty(MyShader& shader)
{
    shader.setMat4("model", getModelMatrix());
}

void GComponent::Model::tick()
{
    //if (!parent_) {
        RenderManager::getInstance().EmplaceRenderCommand(name_, shader_, mesh_);
    //}
#ifdef _DEBUGGing
        std::cout << std::format("OBJ:[{: <25}] | SHADER:[{: <5}] | MESH:[{}]\n", name_, shader_, mesh_);
#endif
   
}

void Model::Draw(MyShader * shader)
{
    shader->setMat4("model", getModelMatrix());
    /* Draw With Source */
}


