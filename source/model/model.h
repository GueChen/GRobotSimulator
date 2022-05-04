#ifndef MODEL_H
#define MODEL_H

#include "component/component.hpp"
#include "manager/modelmanager.h"

#include <string>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

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

using _pModel = shared_ptr<Model>;

class Model
{
    friend class ModelManager;
public:
    explicit Model(Model * parent = nullptr, const string & meshKey = "");
    virtual 
    ~Model();

    virtual 
    void    Draw(MyShader * shader);

    void    setModelMatrix(const mat4 & mat);
    mat4    getModelMatrix() const;

    void    setMesh(const string & mesh);
    string  getMesh() const;

    void    setParent(Model * parent);
    void    appendChild(const _pModel & pchild, mat4 transform = mat4(1.0f));
    const vector<pair<_pModel, mat4>> & getChildren() const;

    void    setAxis(vec3 axis);
    void    setRotate(float angle);

    void    updateChildrenMatrix();
    
protected:
    /// Fields 数据域
    mat4 _parentMatrix = mat4(1.0f);
    mat4 _matrixModel  = mat4(1.0f);

    Model * _parent;
    vector<pair<_pModel, mat4>> _children;
private:
    /// Structure 结构相关
    int ID;
    string _mesh;
    string _shader;

    /// MoveMent 运动相关
    vec3  _axis  = vec3(0.0f, 1.0f, 0.0f);
    float _angle = 0.0;

    // TODO : 将原本的体系进行替换完善
    /*void tick() {};
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
