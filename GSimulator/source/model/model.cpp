#include "model.h"

#include "manager/rendermanager.h"
#include "render/myshader.h"

#include "component/transform_component.h"
#include "component/component_factory.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QjsonArray>

#include <ranges>

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
    if (_RawPtr old_parent = this->parent_; 
        old_parent != nullptr) {
        if (old_parent == parent) return;
        old_parent->eraseChild(old_parent->getChildIndex(this));
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

QJsonObject GComponent::Model::Save()
{
    QJsonObject model_obj;
    model_obj["name"] = QString::fromStdString(name_);
    model_obj["mesh"] = QString::fromStdString(mesh_);

    QJsonArray com_objs;
    for (auto& com : components_ptrs_) com_objs.append(com->Save());
    model_obj["components"] = com_objs;

    QJsonArray children_objs;
    for (auto& child : children_) children_objs.append(child->Save());
    model_obj["children"] = children_objs;

    return model_obj;
}

Model* GComponent::Model::Load(const QJsonObject& obj)
{
    Model* model = new Model;
    
    model->name_ = obj["name"].toString().toStdString();
    model->mesh_ = obj["mesh"].toString().toStdString();

    for (const QJsonValue& json_val : obj["components"].toArray()) {
        //
        QJsonObject com_obj = json_val.toObject();
        std::string type    = com_obj["type"].toString().toStdString();
        if (type != TransformComponent::type_name) {
            std::unique_ptr<Component> com = ComponentFactory::builder[type](model);
            com->Load(com_obj);
            model->RegisterComponent(std::move(com));
        }
        else {
            static_cast<Component*>(model->transform_)->Load(com_obj);
        }
    }

    for (const QJsonValue& child_obj : obj["children"].toArray()) {
        Model* child = Load(child_obj.toObject());
        child->parent_ = model;
    }
        
    return model;
}
