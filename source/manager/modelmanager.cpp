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

ModelManager::~ModelManager() = default;

ModelManager& ModelManager::getInstance() 
{
    static ModelManager instance;
    return instance;
}

bool ModelManager::RegisteredModel(string name, Model *ptr_model)
{   

    ptr_model->model_id_ = next_model_id_;
    ptr_model->name_     = name;

    model_name_to_handle_table_.emplace(name, next_model_id_);
    model_handle_to_name_table_.emplace(next_model_id_, name);
    models_.emplace(next_model_id_, unique_ptr<Model>(ptr_model));

    ++next_model_id_;
    return true;
}

bool ModelManager::DeregisteredModel(size_t model_id)
{
    auto iter = models_.find(model_id);
    if(iter == models_.end()) return false;
    /* Derigister Children */
    for (auto& [child, _] : iter->second->getChildren()) 
    {
        DeregisteredModel(child->model_id_);
    }
    /* Derigister Model */
    string name = model_handle_to_name_table_[model_id];    
    models_.erase(model_id);
    model_handle_to_name_table_.erase(model_id);
    model_name_to_handle_table_.erase(name);
    return true;
}

Model* ModelManager::GetModelByHandle(size_t model_id) const
{
    auto iter = models_.find(model_id);
    if (iter == models_.end()) return nullptr;
    return iter->second.get();
}

Model* ModelManager::GetModelByName(string name) const
{
    auto iter = model_name_to_handle_table_.find(name);
    if (iter == model_name_to_handle_table_.end()) return nullptr;
    return GetModelByHandle(iter->second);
}

bool ModelManager::RegisteredAuxiModel(string name, Model* ptr_model)
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

Model* ModelManager::GetAuxiModelByName(string name) const
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
        model->tick();
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

void ModelManager::SetProjectViewMatrices(glm::mat4 proj, glm::mat4 view)
{
    gl_->setMatrices(matrices_UBO_, proj, view);
}

void ModelManager::SetDirLightViewPosition(glm::vec3 light_dir, glm::vec3 light_color, glm::vec3 view_pos)
{
    gl_->setDirLightViewPos(ambient_observer_UBO_, light_dir, light_color, view_pos);
}

void ModelManager::SetGL(const shared_ptr<MyGL>& gl)
{
    gl_ = gl;
    if (matrices_UBO_) {
        gl_->glDeleteBuffers(1, (unsigned*) & matrices_UBO_);
        gl_->glDeleteBuffers(1, (unsigned*)&ambient_observer_UBO_);
        matrices_UBO_ = 0;
        ambient_observer_UBO_ = 0;
    }
    matrices_UBO_         = gl_->genMatrices();
    ambient_observer_UBO_ = gl_->genDirLightViewPos();
}

/*____________________________________Slots Functions_________________________________________*/
void ModelManager::ResponseDeleteRequest(const string& del_model_name) 
{
    deleted_queue_.push(model_name_to_handle_table_[del_model_name]);  
}

} // namespace GComponent
