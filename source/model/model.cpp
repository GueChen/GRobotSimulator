#include "model.h"

#include "manager/rendermanager.h"
#include "render/myshader.h"

#ifdef _DUBUG
#include <iostream>
#include <format>
#endif

//#define _DEBUG

using namespace GComponent;

Model::Model(_RawPtr parent, const string & meshKey):
    mesh_(meshKey)
{
    if (parent) {
        parent->appendChild(this, model_mat_);
    }
}

Model::Model(const string & name, const string & mesh, const string & shader,const mat4 & model_mat, _RawPtr parent) :
    name_(name), mesh_(mesh), shader_(shader)
{
    setModelMatrix(model_mat);  
    if (parent) {
        parent->appendChild(this, model_mat_);
    }
    updateChildrenMatrix();
}

GComponent::Model::Model(const string & name,  const string & mesh, const string & shader, 
                         const vec3   & trans, const vec3 & rot, const vec3 & scale, _RawPtr parent):
    name_(name), mesh_(mesh), shader_(shader),
    trans_(trans), rot_(rot), scale_(scale)
{    
    updateModelMatrix();
    if (parent) {
        parent->appendChild(this, model_mat_);
    }
    updateChildrenMatrix();
}

Model::~Model() 
{
    if (parent_) 
    {
        parent_->eraseChild(parent_->getChildIndex(this));
    }
    
    parent_ = nullptr;
}

void GComponent::Model::tick(float delta_time)
{
    RenderManager::getInstance().EmplaceRenderCommand(name_, shader_, mesh_);
    for (auto& component : components_ptrs_) {
        component->tick(delta_time);
    }
}

void Model::Draw(MyShader* shader)
{
    shader->setMat4("model", getModelMatrix());
    /* Draw With Source */
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
    trans_     = vec3(model_mat_[3][0], model_mat_[3][1], model_mat_[3][2]);
    rot_       = Conversion::fromVec3f(LogMapSO3Toso3(static_cast<Eigen::Matrix3f>(Conversion::toMat4f(mat).block(0, 0, 3, 3))));
    updateChildrenMatrix();
}

mat4 GComponent::Model::getTranslationModelMatrix() const
{
    mat4 model_mat = getModelMatrix();
    return mat4(1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                model_mat[3][0], model_mat[3][1], model_mat[3][2], 1.0f);
}

mat4 GComponent::Model::getModelMatrixWithoutScale() const
{
    mat4 model_mat = parent_model_mat_;
    model_mat = glm::translate(model_mat, trans_);
    if (float angle = glm::l2Norm(rot_); angle > 1e-6)
    {
        model_mat = glm::rotate(model_mat, angle, rot_ / angle);
    }
    return model_mat;
}

void GComponent::Model::setTransLocal(const vec3& translation, bool updateflag)
{
    trans_ = translation;
    if (updateflag)
    {
        updateModelMatrix();
    }
}

vec3 GComponent::Model::getTransGlobal() const
{
    mat4 model_mat = getModelMatrix();
    return vec3(model_mat[3][0], model_mat[3][1], model_mat[3][2]);
}

void GComponent::Model::setRotLocal(const vec3& rotation, bool updateflag)
{
    rot_ = rotation;
    if (updateflag)
    {
        updateModelMatrix();
    }
}

vec3 GComponent::Model::getRotGlobal() const
{ 
    return Conversion::fromVec3f(LogMapSO3Toso3(static_cast<Eigen::Matrix3f>(Conversion::toMat4f(getModelMatrix()).block(0, 0, 3, 3))));
}

void GComponent::Model::setScale(const vec3 scale, bool updateflag)
{
    scale_ = scale;
    if (updateflag) 
    {
        updateModelMatrix();
    }
}

bool GComponent::Model::RegisterComponent(_PtrComponent&& component_ptr)
{
    components_ptrs_.push_back(std::move(component_ptr));
    return true;
}

bool GComponent::Model::DerigisterComponent()
{
    return false;
}

void Model::setRotate(float angle)
{
    rot_ = glm::radians(angle) * axis_;
    updateModelMatrix();
}

void Model::appendChild(const _RawPtr pchild, mat4 transform)
{
    pchild->setParent(this);
    pchild->setModelMatrix(transform);
    pchild->parent_model_mat_ = getModelMatrix();
    pchild->updateChildrenMatrix();
    children_.emplace_back(pchild);

}

int GComponent::Model::getChildIndex(_RawPtr ptr)
{
    for (int idx = 0; auto & ptr_child : children_) {
        if (ptr_child == ptr) {
            return idx;
        }
        ++idx;
    }
    return -1;
}

void GComponent::Model::updateModelMatrix()
{
    mat4 model(1.0f);
    model = glm::translate(model, trans_);
    if (float angle = glm::l2Norm(rot_); angle > 1e-6)
    {
        model = glm::rotate(model, angle, rot_ / angle);
    }
    model      = glm::scale(model, scale_);
    model_mat_ = model;
    updateChildrenMatrix();
}

void Model::updateChildrenMatrix()
{
    for(auto & child : children_)
    {
        child->parent_model_mat_ = getModelMatrix();
        child->updateChildrenMatrix();
    }
}

void GComponent::Model::setShaderProperty(MyShader& shader)
{
    shader.setMat4("model", getModelMatrix());
}



