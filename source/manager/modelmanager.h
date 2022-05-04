/**
 *  @file   modelmanager.h
 *  @brief  A Manager Class Definition, Rigister all Model Instance in Hash Map.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date   Apri 21, 2022
 **/

#ifndef GCOMPONENT_MODELMANAGER_H
#define GCOMPONENT_MODELMANAGER_H

#include "base/singleton.h"
#include "render/camera.hpp"
#include "render/mygl.hpp"

#include <Eigen/Dense>

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace GComponent {

using std::string;
using std::vector;
using std::unique_ptr;
using std::shared_ptr;
using std::unordered_map;

class Model;

class ModelManager: public SingletonBase<ModelManager>
{
    friend class SingletonBase<ModelManager>;
    NonCoyable(ModelManager)
public:
    virtual ~ModelManager();

    bool        RegisteredModel(string name, Model* ptr_model);
    bool        DeregisteredModel(size_t handle);
    Model*      GetModelByHandle(size_t handle) const;
    Model*      GetModelByName(string name)     const;
    [[no_discard]]
    size_t      RegisteredCamera(glm::vec3 pos = glm::vec3(0.0f, 0.5f, 8.0f));
    void        DeregisterdCamera(size_t handle);
    Camera*     GetCameraByHandle(size_t handle) const;

    void SetProjectViewMatrices(glm::mat4 proj, glm::mat4 view);

    void SetGL(const shared_ptr<MyGL>& gl);

    void tickAll(float delta_time);

protected:
    ModelManager();

private:    
    /* Model Realated Terms 实例管理相关项 */
    size_t                                    next_model_id_              = 0;
    unordered_map<size_t, unique_ptr<Model>>  models_                     = {};
    unordered_map<string, size_t>             model_name_to_handle_table_ = {};
    unordered_map<size_t, string>             model_handle_to_name_table_ = {};

    vector<unique_ptr<Camera>>                cameras_                    = {};

    size_t                                    matrices_UBO_               = 0;
    shared_ptr<MyGL>                          gl_                         = nullptr;
};

} // namespace GComponent

#endif // GCOMPONENT_MODELMANAGER_H
