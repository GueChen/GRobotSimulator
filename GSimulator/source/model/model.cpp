#include "model.h"

#include "manager/rendermanager.h"
#include "render/myshader.h"
#include "component/transform_component.h"

#ifdef _DUBUG
#include <iostream>
#include <format>
#endif

//#define _DEBUG

using namespace GComponent;
using Translation3f = Eigen::Translation3f;
using AngleAxisf    = Eigen::AngleAxisf;

Model::Model(_RawPtr parent, const string & meshKey):    
    mesh_(meshKey), 
    transform_(new TransformComponent(this))
{
    Initialize(parent);
}

Model::Model(const string & name, const string & mesh, const Mat4 & model_mat, _RawPtr parent) :
    name_(name), 
    mesh_(mesh), 
    transform_(new TransformComponent(this, model_mat))
{
    Initialize(parent);
}

GComponent::Model::Model(const string & name,  const string & mesh, 
                         const Vec3   & trans, const Vec3 & rot, const Vec3 & scale, _RawPtr parent):
    name_(name), 
    mesh_(mesh), 
    transform_(new TransformComponent(this, trans, rot, scale))    
{  
    Initialize(parent);
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
    tickImpl(delta_time);
    // tick all components
    for (auto& component : components_ptrs_) {        
        component->tick(delta_time);
    }
    // tick all children
    for (auto& child : children_) {
        child->tick(delta_time);
    }

    transform_->SetIsDirty(false);
}

void GComponent::Model::tickImpl(float delta_time)
{
  
}

bool GComponent::Model::eraseChild(int idx)
{
    if (idx < 0 || idx >= children_.size()) return false;
    children_.erase(children_.begin() + idx);
    return true;
}

void GComponent::Model::setParent(Model* parent)
{
    // remove from old parent
    if (_RawPtr parent = this->parent_; 
        parent != nullptr) {
        if (parent == parent) return;
        parent->eraseChild(parent->getChildIndex(this));
    }    
    parent_ = parent;

    if (parent_) {
        auto p_trans = parent_->GetComponent<TransformComponent>();
        transform_->SetParentMatrix(p_trans->GetModelMatrixWithoutScale());
    }
    else {
        transform_->SetParentMatrix(Mat4::Identity());
    }
}

Component* GComponent::Model::RegisterComponent(_PtrComponent&& component_ptr)
{
    std::erase_if(components_ptrs_, [component_name = component_ptr->GetTypeName()](auto& com) {
        return com->GetTypeName() == component_name;
    });
    component_ptr->SetParent(this);
    components_ptrs_.push_back(std::move(component_ptr));    
    return components_ptrs_.back().get();
}

bool GComponent::Model::DeregisterComponent(const string& component_name)
{
    size_t size = components_ptrs_.size();
    std::erase_if(components_ptrs_, [&component_name](auto& com) {
        return com->GetTypeName() == component_name;
    });
    return size == components_ptrs_.size();
}

void Model::appendChild(const _RawPtr ptr_child, Mat4 transform)
{        
    ptr_child->setParent(this);
    children_.emplace_back(ptr_child);
    ptr_child->GetTransform()->SetModelLocal(transform);    
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

void GComponent::Model::Initialize(Model* parent)
{
    RegisterComponent(std::unique_ptr<TransformComponent>(transform_));
    if (parent) {
        parent->appendChild(this, transform_->GetModelLocal());
    }
}
