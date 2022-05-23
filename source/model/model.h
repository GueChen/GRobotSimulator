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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/glm.hpp>

#include <string>
#include <memory>
#include <unordered_map>

namespace GComponent {

class Model;
class MyGL;
class MyShader;

using std::vector;
using std::string;
using std::pair;
using std::shared_ptr;
using std::unique_ptr;
using Vec3 = Eigen::Vector3f;
using Mat4 = Eigen::Matrix4f;
using vec3 = glm::vec3;
using mat4 = glm::mat4;
using std::unordered_map;

class Model
{
    using _RawPtr = Model*;   
    using _PtrComponent = unique_ptr<Component>;
    friend class ModelManager;
    friend class RenderManager;
public:
    explicit        Model(_RawPtr parent = nullptr, const string & meshKey = "");
                    Model(const string& name, const string& mesh, const string& shader, const mat4& model_mat = mat4(1.0f), _RawPtr parent =nullptr);
                    Model(const string& name, const string& mesh, const string& shader, 
                          const vec3& trans = vec3(0.0f), const vec3& rot = vec3(0.0f), const vec3& scale = vec3(1.0f), 
                          _RawPtr parent = nullptr);
    
    virtual         ~Model();

    virtual
    void            tick(float delta_time);

    virtual 
    void            Draw(MyShader * shader);

    void            appendChild(const _RawPtr pchild, mat4 transform = mat4(1.0f));

    inline const vector<_RawPtr>& 
                    getChildren() const { return children_; }

    bool            eraseChild(int idx);

    [[deprecated("no implementation for glm axis getting")]]
    void            setModelMatrix(const mat4 & mat);
    inline mat4     getModelMatrix() const               { return parent_model_mat_ * model_mat_; }
        
    mat4            getTranslationModelMatrix() const;
    mat4            getModelMatrixWithoutScale()const;
    inline mat4     getParentModelMatrix()      const    { return parent_model_mat_;}

    inline void     setMesh(const string & mesh_name)    { mesh_ = mesh_name;}
    inline string   getMesh()        const               { return mesh_;}

    inline void     setName(const string & name)         { name_ = name;}
    inline string   getName()        const               { return name_;}

    inline void     setShader(const string& shader_name) { shader_ = shader_name;}
    inline string   getShader()      const               { return shader_;}

    inline void     setParent(Model * parent)            { parent_ = parent;}
    inline _RawPtr  getParent()      const               { return parent_;}

    void            setTransLocal(const vec3& translation, bool updateflag = true);
    inline vec3     getTransLocal()  const               { return trans_;}
    vec3            getTransGlobal() const;

    void            setRotLocal(const vec3& rotation, bool updateflag = true);
    inline vec3     getRotLocal()    const               { return rot_;}
    vec3            getRotGlobal()   const;

    void            setScale(const vec3 scale, bool updateflag = true);
    inline vec3     getScale()       const               { return scale_;}
    // TODO: add QR Decomposition then set global scale
    //vec3          getScaleGlobal() const;

    bool            RegisterComponent(_PtrComponent && component_ptr);
    bool            DerigisterComponent();
    template<class _TypeComponent>
    _TypeComponent* GetComponet(const string & component_name);
    
    inline void     setAxis(vec3 axis)                   { axis_ = axis; }
    void            setRotate(float angle);

protected:
    int             getChildIndex(_RawPtr ptr);
    void            updateModelMatrix();
    void            updateChildrenMatrix();
    virtual void    setShaderProperty(MyShader& shader);

protected:
    // Components 组件体系
    vector<_PtrComponent> components_ptrs_;
    vector<string>        components_type_names_;

    /// Transform 变换相关
    vec3 trans_                 = vec3(0.0f);
    vec3 rot_                   = vec3(0.0f);
    vec3 scale_                 = vec3(1.0f);

    /// Fields 数据域
    mat4 parent_model_mat_      = mat4(1.0f);
    mat4 model_mat_             = mat4(1.0f);

    _RawPtr         parent_     = nullptr;
    vector<_RawPtr> children_   = {};

    /// Structure 结构相关
    int     model_id_           = -1;
    string  name_               = "";
    string  mesh_               = "";
    string  shader_             = "";

    // TODO : 将其替换为 Motor/Joint 组件
    /// MoveMent 运动相关
    vec3  axis_                 = vec3(0.0f, 1.0f, 0.0f);
    float angle_                = 0.0;    
};

template<class _TypeComponent>
_TypeComponent* Model::GetComponet(const string& component_name) {
    for (int i = 0; i < components_type_names_.size(); ++i) {
        if (components_type_names_[i] == component_name) {
            return dynamic_cast<_TypeComponent*>(components_ptrs_[i]);
        }
    }
    return nullptr;
}

}

#endif // MODEL_H
