#ifndef MODEL_H
#define MODEL_H

#include "component/component.hpp"

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
using vec3 = glm::vec3;
using mat4 = glm::mat4;
using std::unordered_map;
using _pModel = Model*;
using _Child  = pair<_pModel, mat4>;

class Model
{
    friend class ModelManager;
    friend class RenderManager;
public:
    explicit Model(Model * parent = nullptr, const string & meshKey = "");
    virtual 
    ~Model();

    virtual
    void    tick();

    virtual 
    void    Draw(MyShader * shader);

    void            appendChild(const _pModel& pchild, mat4 transform = mat4(1.0f));
    inline const   
    vector<_Child>& getChildren() const { return children_; }

    bool            eraseChild(int idx);

    void            setModelMatrix(const mat4 & mat);
    inline mat4     getModelMatrix() const               { return parent_model_mat_ * model_mat_; }

    inline void     setMesh(const string & mesh_name)    { mesh_ = mesh_name;}
    inline string   getMesh()        const               { return mesh_;}

    inline void     setName(const string & name)         { name_ = name;}
    inline string   getName()        const               { return name_;}

    inline void     setShader(const string& shader_name) { shader_ = shader_name;}
    inline string   getShader()      const               { return shader_;}

    inline void     setParent(Model * parent)            { parent_ = parent;}
    inline Model*   getParent()      const               { return parent_;}

    inline void     setAxis(vec3 axis)                   { axis_ = axis; }
    void    setRotate(float angle);

protected:
    int     getChildIndex(_pModel ptr);
    void    updateChildrenMatrix();
    virtual void setShaderProperty(MyShader& shader);

protected:
    /// Fields 数据域
    mat4 parent_model_mat_ = mat4(1.0f);
    mat4 model_mat_        = mat4(1.0f);

    _pModel parent_;
    vector<_Child> children_;

    /// Structure 结构相关
    int     model_id_;
    string  name_;
    string  mesh_;
    string  shader_;

    /// MoveMent 运动相关
    vec3  axis_  = vec3(0.0f, 1.0f, 0.0f);
    float angle_ = 0.0;

    // TODO : 将原本的体系进行替换完善
    /*
    void AddComponent(const Component& component);
    void DelComponent(string name);

    size_t            id_;
    string            name_;
    Model* parent_;
    vector<Model*>    children_;
    vector<Component> components_;
    vector<string>    components_type_name_;*/
};

}

#endif // MODEL_H
