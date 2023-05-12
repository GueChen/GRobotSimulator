/**
 *  @file  	model.h
 *  @brief 	the basic model in the simulation software.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @data   OCt 25, 2021
 *  @update May 15, 2022 Modified the Model Fields, add trans, rot, scale
 *       	May 23, 2022 Ready to Add Components Class as Members
 **/

#ifndef MODEL_H
#define MODEL_H

#include "function/conversion.hpp"
#include "component/component.hpp"

#include <GComponent/gtransform.hpp>

#include <string>
#include <memory>
#include <unordered_map>

#include <QtCore/QJsonObject>

namespace GComponent {

class MyGL;
class MyShader;
class TransformComponent;

using std::vector;
using std::string;
using std::pair;
using std::shared_ptr;
using std::unique_ptr;
using std::unordered_map;

using Vec3  = Eigen::Vector3f;
using Vec4  = Eigen::Vector4f;
using Mat3  = Eigen::Matrix3f;
using Mat4  = Eigen::Matrix4f;

class Model
{
/// Type Alias
    using _RawPtr       = Model*;   
    using _PtrComponent = unique_ptr<Component>;

/// Friends
    friend class ModelManager;
    friend class RenderManager;

/// Class Methods
public:
   /// <summary>
   /// no parameter ctor for some special type derived
   /// <para>
   /// 无参构造版本针对一些特殊类型子类
   /// </para>
   /// </summary>
   /// <param name="parent"></param>
   /// <param name="meshKey"></param>
   explicit        Model(_RawPtr parent = nullptr, const string & meshKey = "");

                    /// <summary>
                    /// Create a new {name}ed Model with a specific  bind named {mesh}.                    
                    /// NOTICE: the {model_mat} is relative transform from parent, 
                    /// if no parent exist that means the absolute transform from world coordinates                    
                    /// <para>
                    /// 创建一个新的以 name 命名的 Model 对象，并将名称为 mesh 的网格绑定给它。                   
                    /// 注意：模型矩阵是相对于父对象的相对变换，若无父对象则是相对于世界坐标的绝对变换
                    /// </para>
                    /// </summary>
                    /// <param name="name"></param>
                    /// <param name="mesh"></param>
                    /// <param name="model_mat"></param>
                    /// <param name="parent"></param>
                    Model(const string& name, const string& mesh, const Mat4& model_mat, _RawPtr parent =nullptr);

                    Model(const string& name, const string& mesh,
                          const Vec3& trans = Vec3::Zero(), const Vec3& rot = Vec3::Zero(), const Vec3& scale = Vec3::Ones(),
                          _RawPtr parent = nullptr);
    
    virtual         ~Model();

    void            tick(float delta_time);
    virtual
    void            tickImpl(float delta_time);

    void            appendChild(const _RawPtr pchild, Mat4 transform);

    inline const vector<_RawPtr>& 
                    getChildren() const { return children_; }

    bool            eraseChild(int idx);

    inline void     setMesh(const string & mesh_name)    { mesh_ = mesh_name;}
    inline string   getMesh()        const               { return mesh_;}

    inline void     setName(const string & name)         { name_ = name;}
    inline string   getName()        const               { return name_;}

    void            setParent(Model* parent); 
    inline _RawPtr  getParent()      const               { return parent_;}
   
    // TODO: add QR Decomposition then set global scale
    //vec3          getScaleGlobal() const;

    Component*      RegisterComponent(_PtrComponent && component_ptr);
    bool            DeregisterComponent(const string& component_name);
    inline const vector<_PtrComponent>&
                    GetComponents()  const              { return components_ptrs_;}
    template<class _TypeComponent> requires std::is_base_of_v<Component, _TypeComponent>
    _TypeComponent* GetComponent();
    
    inline TransformComponent* 
                    GetTransform() { return transform_; }
protected:
    int             getChildIndex(_RawPtr ptr);       
    QJsonObject     Save();

private:
    void            Initialize(Model* parent);

/// Fields 数据域
protected:    
    /// Structure 结构相关
    int                     model_id_           = -1;
    string                  name_               = "";
    string                  mesh_               = "";
   
    // Components 组件体系
    vector<_PtrComponent>   components_ptrs_    = {};

    // Relationships 父子关系
    _RawPtr                 parent_             = nullptr;
    vector<_RawPtr>         children_           = {};
    
    /// Transform 变换相关
    TransformComponent*     transform_          = nullptr;
        
#ifdef _COLLISION_TEST
public:
    bool                    intesection_        = false;
#endif // _COLLISION_TEST

};

/// <summary>
/// This functions is used to get the specify Type Component from a model object
/// </summary>
/// <typeparam name="_TypeComponent">component type</typeparam>
/// <param name="component_name">the key to find the component</param>
/// <returns>the specific type component</returns>
template<class _TypeComponent> requires std::is_base_of_v<Component, _TypeComponent>
_TypeComponent* Model::GetComponent() {
    for (int i = 0; i < components_ptrs_.size(); ++i) {
        if (components_ptrs_[i]->GetTypeName() == _TypeComponent::type_name) {
            return dynamic_cast<_TypeComponent*>(components_ptrs_[i].get());
        }
    }
    return nullptr;
}

}

#endif // MODEL_H
