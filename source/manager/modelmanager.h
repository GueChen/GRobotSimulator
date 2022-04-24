/**
 *  @file   modelmanager.h
 *  @brief  A Manager Class Definition, Rigister all Model Instance in Hash Map.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date   Apri 21, 2022
 **/

#ifndef GCOMPONENT_MODELMANAGER_H
#define GCOMPONENT_MODELMANAGER_H

#include "Base/singleton.h"

#include <vector>
#include <memory>
#include <unordered_map>

namespace GComponent {

using std::vector;
using std::unique_ptr;
using std::unordered_map;

class Model;

class ModelManager: SingletonBase<ModelManager>
{
    NonCoyable(ModelManager)
public:
    virtual ~ModelManager();

    bool AddModel(Model* ptr_model);

    bool DelModel(size_t handle);

    void tickAll(float delta_time);
protected:
    ModelManager() = default;

private:
    size_t                                    next_model_id_ = 0;
    unordered_map<size_t, unique_ptr<Model>>  models_;

};

} // namespace GComponent

#endif // GCOMPONENT_MODELMANAGER_H
