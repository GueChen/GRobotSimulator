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

Model::Model(const string & name, const string & mesh, const string & shader,const mat4 & model_mat, Model* parent) :
    parent_(parent), name_(name), mesh_(mesh), shader_(shader), model_mat_(model_mat)
{}

Model::~Model() 
{
    if (parent_) 
    {
        parent_->eraseChild(parent_->getChildIndex(this));
    }
    
    parent_ = nullptr;
}

bool GComponent::Model::eraseChild(int idx)
{
    if (idx < 0 || idx >= children_.size()) return false;
    children_.erase(children_.begin() + idx);
    return true;
}

void Model::setModelMatrix(const mat4 &mat)
{
    model_mat_ = mat;
    updateChildrenMatrix();
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
    pchild->parent_model_mat_ = parent_model_mat_ * model_mat_ * transform;
    pchild->updateChildrenMatrix();
}

int GComponent::Model::getChildIndex(_pModel ptr)
{
    for (int idx = 0; auto & [ptr_child, _] : children_) {
        if (ptr_child == ptr) {
            return idx;
        }
        ++idx;
    }
    return -1;
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
    RenderManager::getInstance().EmplaceRenderCommand(name_, shader_, mesh_);
#ifdef _DEBUGGing
        std::cout << std::format("OBJ:[{: <25}] | SHADER:[{: <5}] | MESH:[{}]\n", name_, shader_, mesh_);
#endif   
}

void Model::Draw(MyShader * shader)
{
    shader->setMat4("model", getModelMatrix());
    /* Draw With Source */
}


