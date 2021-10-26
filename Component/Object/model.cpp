#include "model.h"
#include "glm/gtc/matrix_transform.hpp"
#include <QDebug>

using namespace GComponent;

unordered_map<int, _pModel> Model::table = unordered_map<int, _pModel>();

int Model::generateID()
{
    static int _count = 0;
    return ++_count;
}

Model::Model(Model * parent):
    _parent(parent)
{
    ID = generateID();
}

// FIXME: 可能会出错，需要工厂函数包一下感觉
Model::~Model(){
    //table.erase(table.find(ID));
}

string Model::getMesh()
{
    return _mesh;
}

void Model::setMesh(const string & mesh)
{
    _mesh = mesh;
}

void Model::setShader(const string & shader)
{
    _shader = shader;
}

void Model::setAxis(vec3 axis)
{
    _axis = axis;
}

void Model::setRotate(float angle)
{
    mat4 _mat = glm::identity<mat4>();
    _matrixModel = glm::rotate(_mat, glm::radians(angle), _axis);
    updateChildrenMatrix();
}

void Model::setParent(Model *parent)
{
    _parent = parent;
}

void Model::appendChild(const _pModel &pchild, mat4 transform)
{
    pchild->setParent(this);
    _children.emplace_back(std::make_pair(pchild, transform));
    updateChildrenMatrix();
}

void Model::updateChildrenMatrix()
{
    for(auto & [_child, transf]: _children)
    {
        _child->_parentMatrix = _parentMatrix * _matrixModel * transf;
        _child->updateChildrenMatrix();
    }
}

void Model::Draw()
{
    //TODO: 获取 shader 和 mesh
    //      设置 shader 的 uniform
    //      调用 mesh 的 Draw
}


