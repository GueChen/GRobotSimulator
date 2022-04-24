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

bool ModelManager::AddModel(Model *ptr_model)
{
    unique_ptr<Model> uptr_model(ptr_model);
    models_.emplace(next_model_id_, move(uptr_model));
    ++next_model_id_;
    return true;
}

bool ModelManager::DelModel(size_t model_id)
{
    auto iter = models_.find(model_id);
    if(iter == models_.end()) return false;

    models_.erase(model_id);

    return true;
}

void ModelManager::tickAll(float delta_time)
{
    for(auto && [id, model]: models_){
        // model->tick();
    }
}

} // namespace GComponent
