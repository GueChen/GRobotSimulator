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

class ComponentDeleter {
public:
    ComponentDeleter(Model& model) : model(model) {}

    void Invoke(const string& component_name) {
        model.DeregisterComponent(component_name);
    }
private:
    Model& model;
};

class ModelManager: public QObject
{
    Q_OBJECT

    NonCoyable(ModelManager)
public:
    static      
    ModelManager& getInstance();

    virtual     ~ModelManager();

    bool        RegisteredModel(const string& name, Model* ptr_model);
    bool        DeregisteredModel(size_t handle);
    bool        ChangeModelParent(const string& child, const string& new_parent);
    Model*      GetModelByHandle(size_t handle)            const;
    Model*      GetModelByName(const string& name)         const;
    size_t      GetIDByName(const string& name)            const;
    string      GetNameByID(size_t handle)                 const;
    

    bool        RegisteredAuxiModel(const string& name, Model* ptr_model);
    bool        DeregisteredAuxiModel(size_t handle);
    Model*      GetAuxiModelByHandle(size_t handle)        const;
    Model*      GetAuxiModelByName(const string& name)     const;

    [[nodiscard]]
    size_t      RegisteredCamera(glm::vec3 pos = glm::vec3(5.0f, 0.0f, 1.0f));
    void        DeregisterdCamera(size_t handle);
    Camera*     GetCameraByHandle(size_t handle) const;
   
    void tickAll(float delta_time);

    inline const unordered_map<size_t, string>& GetModelsIDWithName() const { return model_handle_to_name_table_; }
    inline const unordered_map<string, size_t>& GetModelsNameWithID() const { return model_name_to_handle_table_; }

protected:
    ModelManager();

signals:
    void ModelRegisterNotice(const string& model_name, const string& parent_name);
    void ModelDeregisterNotice(const string& model_name);
    void ModelParentChangeNotice(const string& model_name, const string& new_parent_name);

public slots:
    void ResponseDeleteRequest(const string& del_model_name);
    // TODO: 实现该槽函数
    //void ResponseParentChangeRequest(const string& model_name, const string& new_parent_name);

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
       
};

} // namespace GComponent

#endif // GCOMPONENT_MODELMANAGER_H
