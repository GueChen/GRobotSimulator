/**
 *  @file   modelmanager.cpp
 *  @brief  The Implementation of ModelManager Class.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date   Apri 21, 2022
 **/

#include "modelmanager.h"

#include "model/model.h"

namespace GComponent {
using std::move;

ModelManager::ModelManager() = default;

ModelManager::~ModelManager() = default;

bool ModelManager::RegisteredModel(string name, Model *ptr_model)
{

    
    ptr_model->ID = next_model_id_;
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
    return nullptr;
}

void ModelManager::tickAll(float delta_time)
{
    for(auto && [id, model]: models_){
        //model->tick();
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

void ModelManager::SetGL(const shared_ptr<MyGL>& gl)
{
    gl_ = gl;
    if (!matrices_UBO_) {
        matrices_UBO_ = gl_->genMatrices();
    }
}

} // namespace GComponent
