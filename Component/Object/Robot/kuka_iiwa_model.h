#ifndef KUKA_IIWA_MODEL_H
#define KUKA_IIWA_MODEL_H
#include <memory>
#include <array>
#include <eigen3/Eigen/Dense>
#include "GComponent/GTransform.hpp"
#include "Component/Object/model.h"

namespace GComponent{

/// Forward Declarition
class MyGL;
class Mesh;
class Revolute;
class MyShader;

/// STL
using std::array;
using std::vector;
using std::unique_ptr;
using std::shared_ptr;

/// Type Alias
using vec3d         = Vector3d;
using IIWAThetas    = array<double, 7>;
using IIWAThetav    = Matrix<double, 7, 1>;
using IIWATransfoms = array<SE3d, 7>;
using IIWAJacobian  = Matrix<double, 6, 7>;
using IIWAExpCoords = array<twistd, 7>;


class KUKA_IIWA_MODEL:public Model
{
/// 成员函数 Member Functions
public:
/// 构造函数 Constructors
    explicit KUKA_IIWA_MODEL(mat4 transform = mat4(1.0f));
    ~KUKA_IIWA_MODEL() = default;

/// 绘图函数 Drawing Functions
    void Draw(MyShader * shader) override;
    void setColor(const vec3 & color);
    static void setGL(const shared_ptr<MyGL> & other);

/// 运动学函数 Kinematic Functions
    SE3d FowardKinematic();
    SE3d FowardKinematic(const IIWAThetas&);
    SE3d FowardKinematic(const IIWAThetav&);

    IIWATransfoms GetIIWATransforms();
    IIWATransfoms GetIIWATransforms(const IIWAThetas&);
    IIWATransfoms GetIIWATransforms(const IIWAThetav&);

    IIWAThetas BackKinematic(const SE3d&);
    IIWAThetas BackKinematic(const twistd&);
    IIWAThetas BackKinematic(const vec3d&, const vec3d&);

    IIWAJacobian GetJacobian();
    IIWAJacobian GetJacobian(const IIWAThetav&);
    IIWAJacobian GetJacobian(const IIWAThetas&);

/// 操作控制函数 Controller Functions
    void Move(const IIWAThetas&);
    void Move(const IIWAThetav&);

/// 设置获取系列 Setter And Getter
    IIWAThetas GetThetas() const;
    inline void SetThetas(const IIWAThetas&);
private:
    void Draw(MyShader* shader, Model * next);

    void InitCoords();
    void InitializeResource();

    static void InsertMeshResource(const string & key, const string & source);

/// 数据域 Fields
public:
    vector<shared_ptr<Revolute>> Joints;

private:
// 当前单一颜色着色器下的显示颜色
    vec3 _color     = vec3(1.0f);
// 角度及坐标变换参数
    IIWAThetas      _thetas;
    IIWATransfoms   _Ts;

// FIXME：单一窗口下资源优化的最佳方式，但多窗口下可能会存在隐患
/// 资源管理部分 Resource Manager
    static bool hasInit;
    static unordered_map<string, unique_ptr<Mesh>> meshResource;

/// 运动学部分 Kinematic Part
    static SE3d M;
    static IIWAExpCoords expCoords;
    constexpr static unsigned JointNum = 7;
};


}

#endif // KUKA_IIWA_MODEL_H
