#ifndef MODEL_H
#define MODEL_H

// TODO: 单有一个 Mesh 好像也够了
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
public:
    explicit Model(Model * parent = nullptr, const string & meshKey = "");
    virtual ~Model();

    virtual void Draw(MyShader * shader);

    string getMesh() const;
    mat4 getModelMatrix() const;

    const vector<pair<_pModel, mat4>> & getChildren() const;

    void setModelMatrix(const mat4 & mat);
    void setMesh(const string & mesh);

    void setParent(Model * parent);
    void appendChild(const _pModel & pchild, mat4 transform = mat4(1.0f));

    void setAxis(vec3 axis);
    void setRotate(float angle);

    void updateChildrenMatrix();
private:
    static int generateID();

/// Fields 数据域
protected:
    mat4 _parentMatrix = mat4(1.0f);
    mat4 _matrixModel = mat4(1.0f);

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

};

}

#endif // MODEL_H
