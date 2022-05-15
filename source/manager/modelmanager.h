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
#include "model/model.h"

#include <Eigen/Dense>

#include <QtCore/QObject>

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <unordered_map>

namespace GComponent {

using std::string;
using std::vector;
using std::queue;
using std::unique_ptr;
using std::shared_ptr;
using std::unordered_map;

class ModelManager: public QObject
{
    Q_OBJECT

    NonCoyable(ModelManager)
public:
    static      
    ModelManager& getInstance();

    virtual     ~ModelManager();

    bool        RegisteredModel(string name, Model* ptr_model);
    bool        DeregisteredModel(size_t handle);
    Model*      GetModelByHandle(size_t handle)     const;
    Model*      GetModelByName(string name)         const;
    size_t      GetIDByName(const string& name)     const;
    string      GetNameByID(size_t handle)          const;

    bool        RegisteredAuxiModel(string name, Model* ptr_model);
    bool        DeregisteredAuxiModel(size_t handle);
    Model*      GetAuxiModelByHandle(size_t handle) const;
    Model*      GetAuxiModelByName(string name)     const;

    [[nodiscard]]
    size_t      RegisteredCamera(glm::vec3 pos = glm::vec3(0.0f, 0.5f, 8.0f));
    void        DeregisterdCamera(size_t handle);
    Camera*     GetCameraByHandle(size_t handle) const;

    void SetProjectViewMatrices(glm::mat4 proj, glm::mat4 view);
    void SetDirLightViewPosition(glm::vec3 light_dir, glm::vec3 light_color, glm::vec3 view_pos);

    void SetGL(const shared_ptr<MyGL>& gl);

    void tickAll(float delta_time);

    inline const unordered_map<size_t, string>& GetModelsIDWithName() const { return model_handle_to_name_table_; }
    inline const unordered_map<string, size_t>& GetModelsNameWithID() const { return model_name_to_handle_table_; }

protected:
    ModelManager();

signals:
    void ModelRegisterNotice(const string& model_name, const string& parent_name);

public slots:
    void ResponseDeleteRequest(const string& del_model_name);

private:    
    /* Model Realated Terms 实例管理相关项 */
    size_t                                    next_model_id_                        = 1;
    unordered_map<size_t, unique_ptr<Model>>  models_                               = {};
    unordered_map<string, size_t>             model_name_to_handle_table_           = {};
    unordered_map<size_t, string>             model_handle_to_name_table_           = {};

    size_t                                    next_auxiliary_id_                    = 1;
    unordered_map<size_t, unique_ptr<Model>>  auxiliary_models_                     = {};
    unordered_map<string, size_t>             auxiliary_name_to_handle_table_       = {};
    unordered_map<size_t, string>             auxiliary_handle_to_name_table_       = {};
    
    queue<size_t>                             deleted_queue_                         = {};
    /* 值得考虑该三项是否移除或给其它对象保管 */
    vector<unique_ptr<Camera>>                cameras_                              = {};
    size_t                                    matrices_UBO_                         = 0;
    size_t                                    ambient_observer_UBO_                 = 0;
    shared_ptr<MyGL>                          gl_                                   = nullptr;
    
};

} // namespace GComponent

#endif // GCOMPONENT_MODELMANAGER_H
