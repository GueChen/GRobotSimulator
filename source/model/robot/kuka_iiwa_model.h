#ifndef KUKA_IIWA_MODEL_H
#define KUKA_IIWA_MODEL_H

#include "GComponent/GTransform.hpp"

#include "model/model.h"

#include <eigen3/Eigen/Dense>

#include <memory>
#include <array>
#include <map>

namespace GComponent{

/// Forward Declaration
class MeshComponent;
class Revolute;
class MyShader;

/// STL Declaration
using std::map;
using std::array;
using std::vector;
using std::unique_ptr;
using std::shared_ptr;

/// Type Alias
using Weight                = double;
using Radius                = double;
using vec3d                 = Vector3d;
using vec4d                 = Vector4d;
using vec7d                 = Matrix<double, 7, 1>;
using mat6d                 = Matrix<double, 6, 6>;
using mat7d                 = Matrix<double, 7, 7>;
using IIWAThetas            = array<double, 7>;
using IIWAThetav            = Matrix<double, 7, 1>;
using IIWAGradP             = Matrix<double, 7, 3>;
using IIWATransfoms         = array<SE3d, 7>;
using IIWAJacobian          = Matrix<double, 6, 7>;
using PinvJacobian          = Matrix<double, 7, 6>;
using IIWAExpCoords         = array<twistd, 7>;
using BallObstacle          = pair<vec3d, Radius>;
using WeightedCheckPoint    = pair<vec3d, Weight>;

class KUKA_IIWA_MODEL:public Model
{
/// 成员函数 Member Functions
public:
/// 构造函数 Constructors
    explicit KUKA_IIWA_MODEL(mat4 transform = mat4(1.0f));
    ~KUKA_IIWA_MODEL() = default;

/// Tick Functions
    void tick() override;

/// 绘图函数 Drawing Functions
    void Draw(MyShader * shader) override;
    void setColor(const vec3 & color);

/// 运动学函数 Kinematic Functions
    SE3d ForwardKinematic();
    SE3d ForwardKinematic(const IIWAThetas&);
    SE3d ForwardKinematic(const IIWAThetav&);

    IIWATransfoms GetIIWATransforms();
    IIWATransfoms GetIIWATransforms(const IIWAThetas&);
    IIWATransfoms GetIIWATransforms(const IIWAThetav&);

    IIWATransfoms GetIIWATransformsPreSum();
    IIWATransfoms GetIIWATransformsPreSum(const IIWAThetas&);
    IIWATransfoms GetIIWATransformsPreSum(const IIWAThetav&);

    IIWATransfoms GetIIWATransformsDiff();
    IIWATransfoms GetIIWATransformsDiff(const IIWAThetas&);
    IIWATransfoms GetIIWATransformsDiff(const IIWAThetav&);

    IIWAThetas BackKinematic(const SE3d&);
    IIWAThetas BackKinematic(const twistd&);
    IIWAThetas BackKinematic(const vec3d&, const vec3d&);
    IIWAThetas BackKinematic(const SE3d&,  const IIWAThetav&);
    IIWAThetas BackKinematic(const twistd&, const IIWAThetav&);

    IIWAThetas WeightedBackKinematic(const Matrix<double, 7, 7>&, const SE3d&, const IIWAThetav&);
    IIWAThetas WeightedBackKinematic(const Matrix<double, 7, 7>&, const twistd&, const IIWAThetav&);

    IIWAJacobian GetJacobian();
    IIWAJacobian GetJacobian(const IIWAThetav&);
    IIWAJacobian GetJacobian(const IIWAThetas&);

    // 零空间 NullSpace
    mat7d NullSpaceMatrix();
    mat7d NullSpaceMatrix(const IIWAThetav&);
    mat7d NullSpaceMatrix(const IIWAThetas&);

    IIWAThetas SelfMotion(const vec7d& arbVec);
    IIWAThetas SelfMotion(const vec7d& arbVec, const IIWAThetas& thetas);
    IIWAThetas SelfMotion(const vec7d& arbVec, const IIWAThetav& vthetas);

    // 碰撞相关 Collision
    bool CheckCollisionSafe(const vector<BallObstacle>&, const IIWAThetas& thetas);
    bool CheckCollisionSafe(const vector<BallObstacle>&, const IIWAThetav& vthetas);
    bool CheckCollisionSafe(const vector<BallObstacle>&);

    vec7d GetCollisionGrad(const vector<BallObstacle>&, const IIWAThetas& thetas);
    vec7d GetCollisionGrad(const vector<BallObstacle>&, const IIWAThetav& vthetas);
    vec7d GetCollisionGrad(const vector<BallObstacle>&);

    /// 后期删除
    // 仅作测试使用
    double GetCollisionVal(const vector<BallObstacle>&, const IIWAThetas&);
    double GetLimitationVal(const IIWAThetas&);

    // TODO: 把该函数融入连杆的父子关系中，这样可以解决当前 hardcode 的问题
    vector<vec3d> GetCollisionPoints(const IIWAThetas& thetas);
    vector<vec3d> GetCollisionPoints(const IIWATransfoms& preSumT);

    // 关节极限相关 Joint Limitation Relate
    vec7d GetJointsLimitationGrad(const IIWAThetas&);
    vec7d GetJointsLimitationGrad(const IIWAThetav&);
    vec7d GetJointsLimitationGrad();

/// 操作控制函数 Controller Functions
    void Move(const IIWAThetas&);
    void Move(const IIWAThetav&);

/// 设置获取系列 Setter And Getter
    IIWAThetas GetThetas() const;
    void SetThetas(const IIWAThetas&);

    // 添加检测点辅助函数
    void AddCheckPoint(int idx, const WeightedCheckPoint & p);

protected:
    void     setShaderProperty(MyShader & shader) override;
   
private:
    void Draw(MyShader* shader, Model * next);

    void InitializeKinematicsParameters();
    void InitializeResource();
    void InitializeLimitation();

    template<class _LQSolver>
    IIWAThetas BackKinematicIteration(_LQSolver&& solver, const SE3d& trans_desire, const IIWAThetav& initialGuess);
/// 数据域 Fields
public:
    vector<Revolute*> Joints;

private:
// 当前单一颜色着色器下的显示颜色
    vec3 _color     = vec3(1.0f);
// 角度及坐标变换参数
    IIWAThetas      _thetas;
    IIWATransfoms   _Ts;

//  限制参数
    IIWAThetas _thetas_max_limitation;
    IIWAThetas _thetas_min_limitation;

/// 碰撞检测部分 Collision Part
    map<int, vector<WeightedCheckPoint>> _checkPointDict;

// FIXME：单一窗口下资源优化的最佳方式，但多窗口下可能会存在隐患
/// 资源管理部分 Resource Manager
    static bool hasInit;
    static int  count;
/// 运动学部分 Kinematic Part
    static SE3d M;
    static IIWAExpCoords expCoords;
    constexpr static unsigned JointNum = 7;
};


}

#endif // KUKA_IIWA_MODEL_H
