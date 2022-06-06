#include "kuka_iiwa_model.h"

#include "manager/modelmanager.h"
#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"

#include "render/mygl.hpp"
#include "render/myshader.h"
#include "render/rendermesh.h"

#include "component/joint_component.h"
#include "component/joint_group_component.h"

#include "function/adapter/modelloader_qgladapter.h"

#include <GComponent/GNumerical.hpp>
#include <LSSolver/LinearSystemSolver.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <execution>
#include <algorithm>
#include <iostream>
#include <ranges>

#define IIWASource(name) "iiwa14_"#name".STL"

using namespace GComponent;
using std::make_unique;

bool KUKA_IIWA_MODEL::is_init_ = false;
int KUKA_IIWA_MODEL::count    = 0;
SE3d KUKA_IIWA_MODEL::M = SE3d();
array<twistd, 7> KUKA_IIWA_MODEL::expCoords = {};

//vector<vec3d> KUKA_IIWA_MODEL::CollisionCheckPoints = {};

/**
 *      Normal Member Functions
 *            普通成员函数
 ************************************/

/// 资源管理部分
KUKA_IIWA_MODEL::KUKA_IIWA_MODEL(Mat4 transform):
    _thetas({0}), _Ts()
{
    shader_ = "color";
    setModelMatrix(transform); 
    InitializeMeshResource();
    InitializeModelResource();
    InitializeLimitation();
    InitializeKinematicsParameters();
    ++count;
}

void KUKA_IIWA_MODEL::InitializeLimitation(){
    for(auto & maxLimit: _thetas_max_limitation){
        maxLimit = GComponent::MyPI;
    }
    for(auto & minLimit: _thetas_min_limitation){
        minLimit = -GComponent::MyPI;
    }
}

void KUKA_IIWA_MODEL::InitializeKinematicsParameters()
{
    Eigen::Transform<double, 3, Eigen::Affine> M_Ini;
    M_Ini.setIdentity();
    M_Ini.pretranslate(Eigen::Vector3d(0.0f, 0.0f, 1.3320f));
    M = M_Ini.matrix();
//  该部分可改为读取参数文件，或界面接口
    expCoords[0] << 0.,  0., 1.,    0., 0., 0.;
    expCoords[1] << 0.,  1., 0., -0.36, 0., 0.;
    expCoords[2] << 0.,  0., 1.,    0., 0., 0.;
    expCoords[3] << 0., -1., 0.,  0.78, 0., 0.;
    expCoords[4] << 0.,  0., 1.,    0., 0., 0.;
    expCoords[5] << 0.,  1., 0., -1.18, 0., 0.;
    expCoords[6] << 0.,  0., 1.,    0., 0., 0.;
}

void KUKA_IIWA_MODEL::InitializeModelResource()
{            
    ModelManager& model_manager = ModelManager::getInstance();
    array<Model*, 8>    models_tmp;
    array<Vec3, 8> local_trans =
    {
        Vec3(0.0f, 0.0f, 0.0f),
        Vec3(0.0f, 0.0f, 0.1575f),
        Vec3(0.0f, 0.0f, 0.2025f),
        Vec3(0.0f, 0.0f, 0.2045f),
        Vec3(0.0f, 0.0f, 0.2155f),
        Vec3(0.0f, 0.0f, 0.1845f),
        Vec3(0.0f, -0.0607f, 0.2155f),
        Vec3(0.0f, 0.0607f, 0.0809f)
    };

    string count_str = "_" + std::to_string(count);
    model_manager.RegisteredModel("kuka_iiwa_robot" + count_str, this);
    for(int i = 0; i < 8; ++i)
    {
        string count_name = "kuka_iiwa_robot_link_" + std::to_string(i);
        models_tmp[i] = new Model(count_name + count_str, 
                                  count_name, 
                                  "color",
                                  local_trans[i], 
                                  Vec3::Zero(), 
                                  Vec3::Ones(),
                                  i > 0 ? models_tmp[i - 1] :this);
        model_manager.RegisteredModel(models_tmp[i]->getName(), models_tmp[i]);
    }
                
    vector<Vec3> axis_binds = { Vec3::UnitZ(), Vec3::UnitY(),Vec3::UnitZ(), -Vec3::UnitY(), Vec3::UnitZ(), Vec3::UnitY(), Vec3::UnitZ() };
    for (int i = 1; i < 8; ++i) {
        models_tmp[i]->RegisterComponent(make_unique<JointComponent>(models_tmp[i], axis_binds[i - 1]));
    }

    vector<JointComponent*> joints;
    for (int i = 1; i < 8; ++i) {
        joints.push_back(models_tmp[i]->GetComponet<JointComponent>("JointComponent"));
    }
    RegisterComponent(make_unique<JointGroupComponent>(this, joints));             
}

void GComponent::KUKA_IIWA_MODEL::InitializeMeshResource()
{
    if (!is_init_)
    {
        ResourceManager& scene_manager = ResourceManager::getInstance();
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_0", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(base))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_1", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_1))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_2", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_2))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_3", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_3))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_4", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_4))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_5", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_5))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_6", QGL::ModelLoader::getMeshPtr(cPathModel(IIWASource(link_6))));
        scene_manager.RegisteredMesh("kuka_iiwa_robot_link_7", QGL::ModelLoader::getMeshPtr(cPathModel("flanschExten.STL")));
        is_init_ = true;
    }
}

// TODO: 完善 Move
/// 操作控制类函数
void KUKA_IIWA_MODEL::Move(const IIWAThetas& thetas)
{
    SetThetas(thetas);
    JointGroupComponent* joints =  GetComponet<JointGroupComponent>("JointGroupComponent");
    for (int index = 0; auto && joint : joints->GetJoints())
    {
        joint->PushPosBuffer(thetas[index++]);              
    }
}
void KUKA_IIWA_MODEL::Move(const IIWAThetav& vthetas)
{
    SetThetas(IIWAThetas{
                  vthetas[0], vthetas[1], vthetas[2], vthetas[3],
                  vthetas[4], vthetas[5], vthetas[6] });
}

/// 设置获取类
IIWAThetas KUKA_IIWA_MODEL::GetThetas() const
{
    return _thetas;
}

void KUKA_IIWA_MODEL::SetThetas(const IIWAThetas& thetas)
{
    _thetas = thetas;
}

void GComponent::KUKA_IIWA_MODEL::setShaderProperty(MyShader & shader)
{
    shader.setMat4("model", Conversion::fromMat4f(getModelMatrix()));
    shader.setVec3("color", Conversion::fromVec3f(_color));
    shader.setBool("NormReverse", false);
}

void GComponent::KUKA_IIWA_MODEL::tickImpl(float delta_time)
{   
    RenderManager::getInstance().EmplaceRenderCommand(name_, shader_, mesh_);
}

/// 图片绘制部分
void KUKA_IIWA_MODEL::Draw(MyShader * shader)
{
    shader->setVec3("color",Conversion::fromVec3f(_color));
    for (auto& child : children_)
    {
        Draw(shader, child);
    }
}
void KUKA_IIWA_MODEL::Draw(MyShader * shader, Model * next)
{
    shader->setMat4("model", Conversion::fromMat4f(next->getModelMatrix()));
    ResourceManager::getInstance().GetMeshByName(next->getMesh())->Draw();
    for(auto & child : next->getChildren())
    {
        Draw(shader, child);
    }
}

void KUKA_IIWA_MODEL::setColor(const Vec3 &color)
{
    _color = color;
}

/// 运动学部分
/// 获取机器人的变换矩阵
IIWATransfoms KUKA_IIWA_MODEL::GetIIWATransforms()
{
    return GetIIWATransforms(_thetas);
}
IIWATransfoms KUKA_IIWA_MODEL::GetIIWATransforms(const IIWAThetav& vthetas)
{
    return GetIIWATransforms(IIWAThetas{
        vthetas[0], vthetas[1], vthetas[2], vthetas[3],
        vthetas[4], vthetas[5], vthetas[6]});
}
IIWATransfoms KUKA_IIWA_MODEL::GetIIWATransforms(const IIWAThetas& thetas)
{
    IIWATransfoms Ts;
    for (int i = 0; i < JointNum; ++i)
    {
        Ts[i] = ExpMapping(expCoords[i], thetas[i]);
    }
    return Ts;
}

/// 获取变换矩阵前缀积
IIWATransfoms KUKA_IIWA_MODEL::GetIIWATransformsPreSum()
{
    return GetIIWATransformsPreSum(_thetas);
}
IIWATransfoms KUKA_IIWA_MODEL::GetIIWATransformsPreSum(const IIWAThetav & vthetas)
{
    return GetIIWATransformsPreSum(IIWAThetas{
        vthetas[0], vthetas[1], vthetas[2], vthetas[3],
        vthetas[4], vthetas[5], vthetas[6]});
}
IIWATransfoms KUKA_IIWA_MODEL::GetIIWATransformsPreSum(const IIWAThetas & thetas)
{
    IIWATransfoms Ts = GetIIWATransforms(thetas);
    std::partial_sum(Ts.begin(), Ts.end(), Ts.begin(), [](auto&& T1, auto&& T2)
    {
       return T1* T2;
    });
    return Ts;
}

/// 获取变换矩阵微分
IIWATransfoms KUKA_IIWA_MODEL::GetIIWATransformsDiff()
{
    return GetIIWATransformsDiff(_thetas);
}
IIWATransfoms KUKA_IIWA_MODEL::GetIIWATransformsDiff(const IIWAThetav & vthetas)
{
    return GetIIWATransformsDiff(IIWAThetas{
        vthetas[0], vthetas[1], vthetas[2], vthetas[3],
        vthetas[4], vthetas[5], vthetas[6]});
}
IIWATransfoms KUKA_IIWA_MODEL::GetIIWATransformsDiff(const IIWAThetas & thetas)
{
    IIWATransfoms Ts;
    for(int i = 0; i < JointNum; ++i)
    {
        Ts[i] = DiffExpMapping(expCoords[i], thetas[i]);
    }
    return Ts;
}

/// 机器人正运动学
SE3d KUKA_IIWA_MODEL::ForwardKinematic()
{
    return ForwardKinematic(_thetas);
}
SE3d KUKA_IIWA_MODEL::ForwardKinematic(const IIWAThetav& vthetas)
{

    return ForwardKinematic(IIWAThetas{
        vthetas[0], vthetas[1], vthetas[2], vthetas[3],
        vthetas[4], vthetas[5], vthetas[6] });
}
SE3d KUKA_IIWA_MODEL::ForwardKinematic(const IIWAThetas& thetas)
{
    auto Ts = GetIIWATransforms(thetas);
    SE3d T = SE3d::Identity();
    for (int i = 0; i < JointNum; ++i)
    {
        T *= Ts[i];
    }
    return T * M;
}

/// 机器人逆运动学
IIWAThetas KUKA_IIWA_MODEL::BackKinematic(const SE3d& trans_desire)
{
    IIWAThetav thetav = Eigen::Map<IIWAThetav>(_thetas.data(), 7);
    return BackKinematic(trans_desire, thetav);
}
IIWAThetas KUKA_IIWA_MODEL::BackKinematic(const twistd& t_desire)
{
    return BackKinematic(ExpMapping(t_desire));
}
IIWAThetas KUKA_IIWA_MODEL::BackKinematic(const vec3d& pos, const vec3d& ori)
{
    twistd t_desire;
    t_desire.block(0, 0, 3, 1) = ori;
    t_desire.block(3, 0, 3, 1) = pos;
    return BackKinematic(ExpMapping(t_desire));
}
IIWAThetas KUKA_IIWA_MODEL::BackKinematic(const twistd& t_desire, const IIWAThetav& initialGuess){
    return BackKinematic(ExpMapping(t_desire), initialGuess);
}
IIWAThetas KUKA_IIWA_MODEL::BackKinematic(const SE3d& trans_desire, const IIWAThetav& initialGuess){
    DynamicLeastNormSolver<double> LNSolver;
    return BackKinematicIteration(LNSolver, trans_desire, initialGuess);
}

IIWAThetas KUKA_IIWA_MODEL::WeightedBackKinematic(const Matrix<double, 7, 7> & weighted_matrix, const twistd & twist_desire, const IIWAThetav& initial_guess){
    return WeightedBackKinematic(weighted_matrix, ExpMapping(twist_desire), initial_guess);
}
IIWAThetas KUKA_IIWA_MODEL::WeightedBackKinematic(const Matrix<double, 7, 7> & weighted_matrix, const SE3d & trans_desire, const IIWAThetav& initial_guess){
    DynamicWeightedLeastNormSolver<double> WLNSolver(weighted_matrix);
    return BackKinematicIteration(WLNSolver, trans_desire, initial_guess);
}

template<class _LQSolver>
IIWAThetas KUKA_IIWA_MODEL::BackKinematicIteration(_LQSolver && solver, const SE3d& trans_desire, const IIWAThetav& initial_guess){
    IIWAThetav thetav = initial_guess;
    SE3d trans_cur  = ForwardKinematic(thetav);

    /* 注释部分是 左差运算 现采用右差运算，可以省去一次伴随变换 */
    //twistd t_delta  = LogMapSE3Tose3(InverseSE3(trans_cur) * trans_desire);
    twistd t_delta  = LogMapSE3Tose3((trans_desire * InverseSE3(trans_cur)).eval());
    double residual = t_delta.norm();
    int iter = 0;
    double decay = 1;
    double residualDelta = -1e2;
    while (residual > 1e-5 && iter < 50) {

        /* Get Precondition */
        auto t_cal = decay * t_delta;
        twistd Vs = t_cal;

        /* Test new Solution is efficient */
        IIWAThetav thetav_new       = thetav +  solver(GetJacobian(thetav), Vs);
        SE3d       trans_new        = ForwardKinematic(thetav_new);
        twistd     t_delta_new      = LogMapSE3Tose3((trans_desire * InverseSE3(trans_new)).eval());
        double     residual_new     = t_delta_new.norm();

        if (residual - residual_new < residualDelta)
        {
            decay *= 0.3f;
            continue;
        }
        if(decay < 1.0f)
            decay *= 3.333333333333f;

        /* Update Value */
        thetav    = thetav_new;
        t_delta   = t_delta_new;
        trans_cur = trans_new;
        residual  = residual_new;

        if(residual < 10)
            residualDelta = -1e1;
        else if(residual < 1)
            residualDelta = -1;

        iter++;
    }
    std::for_each(std::execution::par, 
                  thetav.begin(), thetav.end(), 
                  [](auto& val) { val = ToStandarAngle(val); });
    return IIWAThetas{ thetav[0], thetav[1], thetav[2], thetav[3],
                thetav[4], thetav[5], thetav[6]};
}

/// 获取机器人的雅可比
IIWAJacobian KUKA_IIWA_MODEL::GetJacobian(const IIWAThetas& thetas)
{
    IIWAJacobian J;
    IIWATransfoms Ts = GetIIWATransforms(thetas);
    SE3d Trans = SE3d::Identity();
    for (int i = 0; i < JointNum; ++i)
    {
        J.block(0, i, 6, 1) = Adjoint(Trans) * expCoords[i];
        Trans *= Ts[i];
    }
    return J;
}
IIWAJacobian KUKA_IIWA_MODEL::GetJacobian(const IIWAThetav& vthetas)
{
    return GetJacobian(IIWAThetas{
        vthetas[0], vthetas[1], vthetas[2], vthetas[3],
        vthetas[4], vthetas[5], vthetas[6]
        });
}
IIWAJacobian KUKA_IIWA_MODEL::GetJacobian()
{
    return GetJacobian(_thetas);
}

/// 获取机器人的零空间矩阵
mat7d KUKA_IIWA_MODEL::NullSpaceMatrix(const IIWAThetas& thetas)
{
    IIWAJacobian J     = GetJacobian(thetas);

    // 通过 SVD 分解求解 Jx = I
    Eigen::JacobiSVD svd(J, Eigen::ComputeFullU|Eigen::ComputeFullV);
    PinvJacobian pinvJ = svd.solve(mat6d::Identity());

    return (mat7d::Identity() - pinvJ * J);
}
mat7d KUKA_IIWA_MODEL::NullSpaceMatrix(const IIWAThetav &vthetas)
{
    return NullSpaceMatrix(IIWAThetas{
        vthetas[0], vthetas[1], vthetas[2], vthetas[3],
        vthetas[4], vthetas[5], vthetas[6]
        });
}
mat7d KUKA_IIWA_MODEL::NullSpaceMatrix()
{
    return NullSpaceMatrix(_thetas);
}

/// 机器人零空间的自运动
IIWAThetas KUKA_IIWA_MODEL::SelfMotion(const vec7d &arbVec, const IIWAThetas& thetas)
{
    Matrix<double, 7, 7> zeroOrthoMat = NullSpaceMatrix(thetas);
    IIWAThetav delta = zeroOrthoMat * arbVec;
    IIWAThetas thetas_ret = _thetas;
    for(int i = 0; i < JointNum; ++i)
    {
        thetas_ret[i] += delta(i, 0);
    }
    return thetas_ret;
}
IIWAThetas KUKA_IIWA_MODEL::SelfMotion(const vec7d &arbVec, const IIWAThetav &vthetas)
{
    IIWAThetas thetaInput = {vthetas[0], vthetas[1], vthetas[2], vthetas[3],
                             vthetas[4], vthetas[5], vthetas[6]};
    return SelfMotion(arbVec, thetaInput);
}
IIWAThetas KUKA_IIWA_MODEL::SelfMotion(const vec7d& arbVec)
{
    return SelfMotion(arbVec, _thetas);
}

/// 连杆中部碰撞点检测，复杂度 O(mn)
bool KUKA_IIWA_MODEL::CheckCollisionSafe(const vector<BallObstacle>& obsts, const IIWAThetas& thetas)
{
    constexpr double padding = 0.05;
    auto CheckPoints = GetCollisionPoints(thetas);
    for(auto && [coc, radius]: obsts)
    {
        for(auto && point: CheckPoints)
        {
            if((coc-point).norm() < radius + padding)
            {
                return true;
            }
        }
    }
    return false;
}
bool KUKA_IIWA_MODEL::CheckCollisionSafe(const vector<BallObstacle> &obsts, const IIWAThetav& vthetas)
{
    return CheckCollisionSafe(obsts,
                              IIWAThetas{vthetas[0], vthetas[1], vthetas[2], vthetas[3],
                                         vthetas[4], vthetas[5], vthetas[6]});
}
bool KUKA_IIWA_MODEL::CheckCollisionSafe(const vector<BallObstacle> &obsts)
{
    return CheckCollisionSafe(obsts, _thetas);
}

/// 获取 碰撞距离函数梯度
/// 复杂度 O(mnl) n 自由度 m 最大检测点数 l 碰撞点数
vec7d KUKA_IIWA_MODEL::GetCollisionGrad(const vector<BallObstacle>& obsts, const IIWAThetas& thetas)
{
    IIWATransfoms preTs       = GetIIWATransformsPreSum(thetas);                    // 前缀和方便后续计算    Get the T_0_i total
    IIWATransfoms dTSingle    = GetIIWATransformsDiff(thetas);                      // 单个变换矩阵微分形式  Get the dTi
    vec7d retGrad             = vec7d::Zero();                                      // 避碰函数梯度          Return value

    for(auto && [index, points] : _checkPointDict){                                 // 遍历检测点字典
        IIWATransfoms dTs;
        for(int i = 0; i < JointNum; ++i){                                          // 求取当前关节下的      parital T
            if(i < index){
                if(i > 0) {
                   dTs[i] = preTs[i - 1];
                }
                dTs[i] = dTSingle[i] * InverseSE3(preTs[i]) * preTs[index];
            }
            else{
                dTs[i] = Matrix4d::Zero();
            }
        }

        for(auto && [point, weight]: points){                                        // 对所有点进行求取
            Matrix<double, 7, 3> pGradMatrix = Matrix<double, 7, 3>::Zero();         // 计算 partial P
            for(int i = 0; i < JointNum; ++i){
                pGradMatrix.row(i) = affineProduct(dTs[i], point).transpose();
            }

            vec3d curPos = affineProduct(preTs[index], point);
            for(auto && [center, radObst]: obsts){                                   // 对每个碰撞点求当前梯度值
                retGrad += weight * pGradMatrix * (curPos - center);
            }
        }
    }

    return retGrad;
}


vec7d KUKA_IIWA_MODEL::GetCollisionGrad(const vector<BallObstacle> &obsts, const IIWAThetav &vthetas)
{
    return GetCollisionGrad(obsts,
            IIWAThetas{vthetas[0], vthetas[1], vthetas[2], vthetas[3],
                    vthetas[4], vthetas[5], vthetas[6]}
    );
}
vec7d KUKA_IIWA_MODEL::GetCollisionGrad(const vector<BallObstacle> &obsts)
{
    return GetCollisionGrad(obsts,
                            _thetas);
}

/// 获取碰撞函数的值
double KUKA_IIWA_MODEL::GetCollisionVal(const vector<BallObstacle>& obsts, const IIWAThetas& thetas)
{
    IIWATransfoms preTs = GetIIWATransformsPreSum(thetas);
    double ret_Val = 0.0;
    for(auto && [index, points]: _checkPointDict){
        for(auto && [point, weight]: points){
            vec3d curPos = affineProduct(preTs[index], point);
            for(auto && [coc, radius]: obsts)
            {
               ret_Val += weight * (curPos-coc).squaredNorm();
            }
        }
    }
    return 0.5 * ret_Val;
}

double KUKA_IIWA_MODEL::GetLimitationVal(const IIWAThetas & thetas){
    double val = 0.0;
    for(int i = 0; i < JointNum; ++i){
        double divisor = std::pow(_thetas_max_limitation[i]- _thetas_min_limitation[i], 2),
               dividend= (_thetas_max_limitation[i] - thetas[i]) * (thetas[i] - _thetas_min_limitation[i]);
        val += divisor/ dividend;
    }
    val *= 0.25;
    return val;
}

/// 获取碰撞点
vector<vec3d> KUKA_IIWA_MODEL::GetCollisionPoints(const IIWAThetas& thetas)
{
    IIWATransfoms Ts = GetIIWATransformsPreSum(thetas);
    return GetCollisionPoints(Ts);
}
vector<vec3d> KUKA_IIWA_MODEL::GetCollisionPoints(const IIWATransfoms &preSumTs)
{
    vector<vec3d> Ret_Points;
    for(auto && [index, points]: _checkPointDict){
        for(auto && [point, weight]: points){
            Ret_Points.emplace_back(affineProduct(preSumTs[index], point));
        }
    }
    return Ret_Points;
}

/// 获取关节极限函数优化解
vec7d KUKA_IIWA_MODEL::GetJointsLimitationGrad(const IIWAThetav & vthetas){
    return GetJointsLimitationGrad(
                IIWAThetas{vthetas[0], vthetas[1], vthetas[2], vthetas[3],
                        vthetas[4], vthetas[5], vthetas[6]});
}
vec7d KUKA_IIWA_MODEL::GetJointsLimitationGrad(){
    return GetJointsLimitationGrad(GetThetas());
}
vec7d KUKA_IIWA_MODEL::GetJointsLimitationGrad(const IIWAThetas & thetas)
{
    vec7d grad;
    for(int i = 0; i < JointNum; ++i){
        double divisor =std::pow(_thetas_max_limitation[i] - _thetas_min_limitation[i], 2) *
                        (2 * thetas[i] - _thetas_max_limitation[i] - _thetas_min_limitation[i]),
               dividend= 4.0 *
                         std::pow(_thetas_max_limitation[i]- thetas[i], 2) *
                         std::pow(thetas[i] - _thetas_min_limitation[i], 2);
        grad[i] = divisor / dividend;
    }
    return grad;
}

void KUKA_IIWA_MODEL::AddCheckPoint(int idx, const WeightedCheckPoint &p)
{
    _checkPointDict[idx].emplace_back(p);
}


