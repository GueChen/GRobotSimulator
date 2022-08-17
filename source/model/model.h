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

#include <GComponent/GTransform.hpp>

#include <string>
#include <memory>
#include <unordered_map>

namespace GComponent {

class MyGL;
class MyShader;

using std::vector;
using std::string;
using std::pair;
using std::shared_ptr;
using std::unique_ptr;
using std::unordered_map;

using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;
using Mat3 = Eigen::Matrix3f;
using Mat4 = Eigen::Matrix4f;

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
    explicit        Model(_RawPtr parent = nullptr, const string & meshKey = "");
                    Model(const string& name, const string& mesh, const string& shader, const Mat4& model_mat, _RawPtr parent =nullptr);
                    Model(const string& name, const string& mesh, const string& shader, 
                          const Vec3& trans = Vec3::Zero(), const Vec3& rot = Vec3::Zero(), const Vec3& scale = Vec3::Ones(),
                          _RawPtr parent = nullptr);
    
    virtual         ~Model();

    void            tick(float delta_time);
    virtual
    void            tickImpl(float delta_time);

    void            appendChild(const _RawPtr pchild, Mat4 transform = Mat4::Identity());

    inline const vector<_RawPtr>& 
                    getChildren() const { return children_; }

    bool            eraseChild(int idx);

    [[deprecated("no implementation for glm axis getting")]]
    void            setModelMatrix(const Mat4 & mat);
    inline Mat4     getModelMatrix() const               { return parent_model_mat_ * model_mat_; }
        
    Mat4            getTranslationModelMatrix() const;
    Mat4            getModelMatrixWithoutScale()const;
    inline Mat4     getParentModelMatrix()      const    { return parent_model_mat_;}

    inline void     setMesh(const string & mesh_name)    { mesh_ = mesh_name;}
    inline string   getMesh()        const               { return mesh_;}

    inline void     setName(const string & name)         { name_ = name;}
    inline string   getName()        const               { return name_;}

    inline void     setShader(const string& shader_name) { shader_ = shader_name;}
    inline string   getShader()      const               { return shader_;}

    inline void     setParent(Model * parent)            { parent_ = parent;}
    inline _RawPtr  getParent()      const               { return parent_;}

    void            setTransLocal(const Vec3& translation, bool updateflag = true);
    inline Vec3     getTransLocal()  const               { return trans_;}
    Vec3            getTransGlobal() const;

    void            setRotLocal(const Vec3& rotation, bool updateflag = true);
    inline Vec3     getRotLocal()    const               { return rot_;}
    Vec3            getRotGlobal()   const;

    void            setScale(const Vec3 scale, bool updateflag = true);
    inline Vec3     getScale()       const               { return scale_;}
    // TODO: add QR Decomposition then set global scale
    //vec3          getScaleGlobal() const;

    bool            RegisterComponent(_PtrComponent && component_ptr);
    bool            DeregisterComponent(const string& component_name);
    inline const vector<_PtrComponent>&
                    GetComponents()  const              { return components_ptrs_;}
    template<class _TypeComponent> requires std::is_base_of_v<Component, _TypeComponent>
    _TypeComponent* GetComponent(const string & component_name);
    
protected:
    int             getChildIndex(_RawPtr ptr);
    void            updateModelMatrix();
    void            updateChildrenMatrix(const Mat3& parent_scale_mat);
    virtual void    setShaderProperty(MyShader& shader);

/// Fields 数据域
protected:
    // Components 组件体系
    vector<_PtrComponent>   components_ptrs_        = {};

    /// Transform 变换相关
    Vec3                    trans_                  = Vec3::Zero();
    Vec3                    rot_                    = Vec3::Zero();
    Vec3                    scale_                  = Vec3::Ones();
    Vec3                    shear_                  = Vec3::Zero();
    Mat4                    parent_model_mat_       = Mat4::Identity();    
    Mat4                    model_mat_              = Mat4::Identity();
    Mat3                    inv_parent_U_mat_       = Mat3::Identity();

    // Relationships 父子关系
    _RawPtr                 parent_                 = nullptr;
    vector<_RawPtr>         children_               = {};

    /// Structure 结构相关
    int                     model_id_               = -1;
    string                  name_                   = "";
    string                  mesh_                   = "";
    string                  shader_                 = "";
};

/// <summary>
/// This functions is used to get the specify Type Component from a model object
/// </summary>
/// <typeparam name="_TypeComponent">component type</typeparam>
/// <param name="component_name">the key to find the component</param>
/// <returns>the specify type component</returns>
template<class _TypeComponent> requires std::is_base_of_v<Component, _TypeComponent>
_TypeComponent* Model::GetComponent(const string& component_name) {
    for (int i = 0; i < components_ptrs_.size(); ++i) {
        if (components_ptrs_[i]->GetTypeName() == component_name) {
            return dynamic_cast<_TypeComponent*>(components_ptrs_[i].get());
        }
    }
    return nullptr;
}

}

#endif // MODEL_H
