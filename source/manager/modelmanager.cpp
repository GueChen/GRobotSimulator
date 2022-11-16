/**
 *  @file   modelmanager.cpp
 *  @brief  The Implementation of ModelManager Class.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date   Apri 21, 2022
 **/

#include "manager/modelmanager.h"

#include <iostream>
#include <format>

//#define _DEBUG

namespace GComponent {
using std::move;

ModelManager::ModelManager() = default;


ModelManager::~ModelManager() {
    auxiliary_models_.clear();
    models_.clear();
    cameras_.clear();
}

ModelManager& ModelManager::getInstance() 
{
    static ModelManager instance;
    return instance;
}

bool ModelManager::RegisteredModel(const string& name, Model *ptr_model)
{   

    ptr_model->model_id_ = next_model_id_;
    ptr_model->name_     = name;

    model_name_to_handle_table_.emplace(name, next_model_id_);
    model_handle_to_name_table_.emplace(next_model_id_, name);
    models_.emplace(next_model_id_, unique_ptr<Model>(ptr_model));    
    ++next_model_id_;
    emit ModelRegisterNotice(name, ptr_model->parent_ ? ptr_model->parent_->name_ : "");
    return true;
}

bool ModelManager::DeregisteredModel(size_t model_id)
{
    auto iter = models_.find(model_id);
    if(iter == models_.end()) return false;

    /* Derigister all Children */  
    vector<int> children_id;
    for (auto& child : iter->second->getChildren()) 
    {
        children_id.push_back(child->model_id_);       
    }
    for (auto& id : children_id) {
        DeregisteredModel(id);
    }

    /* Derigister Model */
    string name = model_handle_to_name_table_[model_id];    
    models_.erase(model_id);
    model_handle_to_name_table_.erase(model_id);
    model_name_to_handle_table_.erase(name);
    emit ModelDeregisterNotice(name);
    return true;
}

Model* ModelManager::GetModelByHandle(size_t model_id) const
{
    auto iter = models_.find(model_id);
    if (iter == models_.end()) return nullptr;
    return iter->second.get();
}

Model* ModelManager::GetModelByName(const string& name) const
{
    auto iter = model_name_to_handle_table_.find(name);
    if (iter == model_name_to_handle_table_.end()) return nullptr;
    return GetModelByHandle(iter->second);
}

size_t ModelManager::GetIDByName(const string& name) const
{
    auto iter = model_name_to_handle_table_.find(name);
    if (iter == model_name_to_handle_table_.end()) return 0;
    return iter->second;
}

string ModelManager::GetNameByID(size_t handle) const
{
    auto iter = model_handle_to_name_table_.find(handle);
    if (iter == model_handle_to_name_table_.end()) return "";
    return iter->second;
}

bool ModelManager::ChangeModelParent(const string& child, const string& new_parent)
{
    if (!model_name_to_handle_table_.count(child) || (!new_parent.empty() && !model_name_to_handle_table_.count(new_parent))) return false;
    emit ModelParentChangeNotice(child, new_parent);
    return true;
}

bool ModelManager::RegisteredAuxiModel(const string& name, Model* ptr_model)
{
    ptr_model->model_id_ = next_auxiliary_id_;
    ptr_model->name_ = name;

    auxiliary_name_to_handle_table_.emplace(name, next_auxiliary_id_);
    auxiliary_handle_to_name_table_.emplace(next_auxiliary_id_, name);
    auxiliary_models_.emplace(next_auxiliary_id_, unique_ptr<Model>(ptr_model));

    ++next_auxiliary_id_;
    return true;
}

bool ModelManager::DeregisteredAuxiModel(size_t model_id)
{
    auto iter = auxiliary_models_.find(model_id);
    if (iter == auxiliary_models_.end()) return false;
    string name = auxiliary_handle_to_name_table_[model_id];

    auxiliary_models_.erase(model_id);
    auxiliary_handle_to_name_table_.erase(model_id);
    auxiliary_name_to_handle_table_.erase(name);

    return true;
}

Model* ModelManager::GetAuxiModelByHandle(size_t model_id) const
{
    auto iter = auxiliary_models_.find(model_id);
    if (iter == auxiliary_models_.end()) return nullptr;
    return iter->second.get();
}

Model* ModelManager::GetAuxiModelByName(const string& name) const
{
    auto iter = auxiliary_name_to_handle_table_.find(name);
    if (iter == auxiliary_name_to_handle_table_.end()) return nullptr;
    return GetAuxiModelByHandle(iter->second);
}

void ModelManager::tickAll(float delta_time)
{

    /* Deleted All Requester Models */
    while (!deleted_queue_.empty()) 
    {
        DeregisteredModel(deleted_queue_.front());
        deleted_queue_.pop();
    }
    /* Tick The Rest Models */
    for(auto && [id, model]: models_)
    {
        if (not model->parent_) {
            model->tick(delta_time);
        }
    }
}

size_t ModelManager::RegisteredCamera(glm::vec3 pos)
{
    cameras_.emplace_back(std::make_unique<Camera>(pos));
    return cameras_.size();
}

void ModelManager::DeregisterdCamera(size_t handle)
{
    if (handle <= cameras_.size())
    {
        cameras_.erase(cameras_.begin() + handle - 1);
    }
}

Camera* ModelManager::GetCameraByHandle(size_t handle) const
{
    if (handle <= cameras_.size()) return cameras_[handle - 1].get();
    return nullptr;
}

/*____________________________________Slots Functions_________________________________________*/
void ModelManager::ResponseDeleteRequest(const string& del_model_name) 
{
    deleted_queue_.push(model_name_to_handle_table_[del_model_name]);  
}

void ModelManager::ResponseParentChangeRequest(const string& model_name, const string& new_parent_name)
{
    Model* model  = GetModelByName(model_name), 
         * parent = GetModelByName(new_parent_name);
    Mat4 ori_mat  = model->getModelMatrix();
    if (parent) {
        Mat4 p_mat = parent->getModelMatrixWithoutScale();
        parent->appendChild(model, InverseSE3(p_mat) * ori_mat);
    }
    else {
        model->setParent(nullptr);
        model->setModelMatrix(ori_mat, false);
        model->updateModelMatrix();
    }
}


} // namespace GComponent
