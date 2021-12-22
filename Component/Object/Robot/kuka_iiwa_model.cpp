#include <glm/gtc/matrix_transform.hpp>
#include "Component/MyGL.hpp"
#include "kuka_iiwa_model.h"
#include "Component/Geometry/mesh.h"
#include "Component/Geometry/modelloader.h"
#include "Component/Object/Robot/joint.h"
#include "Component/myshader.h"
#include <GComponent/GNumerical.hpp>
#include <ranges>
#include <iostream>

#define IIWASource(name) ("iiwa14_"#name".STL")

using namespace GComponent;

bool KUKA_IIWA_MODEL::hasInit = false;
unordered_map<string, unique_ptr<Mesh>> KUKA_IIWA_MODEL::meshResource{};

SE3d KUKA_IIWA_MODEL::M = SE3d();
array<twistd, 7> KUKA_IIWA_MODEL::expCoords = {};


/**
 *      Static Member Functions
 *            静态成员函数
 ************************************/

void KUKA_IIWA_MODEL::setGL(const shared_ptr<MyGL> &other)
{
    for(auto & [_, mesh]: meshResource)
    {
        mesh->setGL(other);
    }
}

void KUKA_IIWA_MODEL::InsertMeshResource(const string &key, const string &source)
{
    meshResource.insert(std::make_pair(
                            key,
                            std::make_unique<Mesh>( ModelLoader::getMesh( sPathModel(source)))));
}


/**
 *      Normal Member Functions
 *            普通成员函数
 ************************************/

/// 资源管理部分
KUKA_IIWA_MODEL::KUKA_IIWA_MODEL(mat4 transform):
    _thetas({0}), _Ts()
{
    _matrixModel = transform;
    InitializeResource();
    std::cout << "Expcoords 0:=\n"  << expCoords[0] << "\n\n";
}

void KUKA_IIWA_MODEL::InitCoords()
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

void KUKA_IIWA_MODEL::InitializeResource()
{

    if(!hasInit)
    {
        InsertMeshResource("Base",  IIWASource(base));
        InsertMeshResource("Link1", IIWASource(link_1));
        InsertMeshResource("Link2", IIWASource(link_2));
        InsertMeshResource("Link3", IIWASource(link_3));
        InsertMeshResource("Link4", IIWASource(link_4));
        InsertMeshResource("Link5", IIWASource(link_5));
        InsertMeshResource("Link6", IIWASource(link_6));
        InsertMeshResource("Flansch", "flanschExten.STL");
        InitCoords();
    }

    vector<shared_ptr<Model>> models(7);
    for(int i = 0; i < 7; ++i)
    {
        models[i] = std::make_shared<Model>();
    }

    setMesh("Base");
    models[0]->setMesh("Link1");
    models[1]->setMesh("Link2");
    models[2]->setMesh("Link3");
    models[3]->setMesh("Link4");
    models[4]->setMesh("Link5");
    models[5]->setMesh("Link6");
    models[6]->setMesh("Flansch");

    models[0]->setAxis(vec3(0.0f, 1.0f, 0.0f));
    models[1]->setAxis(vec3(0.0f, 0.0f, 1.0f));
    models[2]->setAxis(vec3(0.0f, 1.0f, 0.0f));
    models[3]->setAxis(vec3(0.0f, 0.0f, -1.0f));
    models[4]->setAxis(vec3(0.0f, 1.0f, 0.0f));
    models[5]->setAxis(vec3(0.0f, 0.0f, 1.0f));
    models[6]->setAxis(vec3(0.0f, 1.0f, 0.0f));

    mat4 im = glm::identity<mat4>();

    models[5]->appendChild(models[6], glm::translate(im, vec3(0.0f, 0.0809f, 0.0607f)));
    models[4]->appendChild(models[5], glm::translate(im, vec3(0.0f, 0.2155f, -0.0607f)));
    models[3]->appendChild(models[4], glm::translate(im, vec3(0.0f, 0.1845f, 0.0f)));
    models[2]->appendChild(models[3], glm::translate(im, vec3(0.0f, 0.2155f, 0.0f)));
    models[1]->appendChild(models[2], glm::translate(im, vec3(0.0f, 0.2045f, 0.0f)));
    models[0]->appendChild(models[1], glm::translate(im, vec3(0.0f, 0.2025f, 0.0f)));
    appendChild(models[0], glm::translate(im, vec3(0.0f, 0.1575f, 0.0f)));


    Joints.resize(7);
    Joints[0] = std::make_shared<Revolute>(models[0]);
    Joints[1] = std::make_shared<Revolute>(models[1]);
    Joints[2] = std::make_shared<Revolute>(models[2]);
    Joints[3] = std::make_shared<Revolute>(models[3]);
    Joints[4] = std::make_shared<Revolute>(models[4]);
    Joints[5] = std::make_shared<Revolute>(models[5]);
    Joints[6] = std::make_shared<Revolute>(models[6]);

    hasInit = true;
}

/// 图片绘制部分
void KUKA_IIWA_MODEL::Draw(MyShader * shader)
{
    shader->setVec3("color", _color);
    Draw(shader, this);
}
void KUKA_IIWA_MODEL::Draw(MyShader * shader, Model * next)
{
    shader->setMat4("model", next->getModelMatrix());
    meshResource[next->getMesh()]->Draw();
    for(auto & [_child, _]: next->getChildren())
    {
        Draw(shader, _child.get());
    }
}

void KUKA_IIWA_MODEL::setColor(const vec3 &color)
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

/// 机器人正运动学
SE3d KUKA_IIWA_MODEL::FowardKinematic()
{
    return FowardKinematic(_thetas);
}
SE3d KUKA_IIWA_MODEL::FowardKinematic(const IIWAThetav& vthetas)
{
    return FowardKinematic(IIWAThetas{
        vthetas[0], vthetas[1], vthetas[2], vthetas[3],
        vthetas[4], vthetas[5], vthetas[6] });
}
SE3d KUKA_IIWA_MODEL::FowardKinematic(const IIWAThetas& thetas)
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
    SE3d trans_cur  = FowardKinematic(thetav);
    twistd t_delta  = LogMapSE3Tose3(InverseSE3(trans_cur) * trans_desire);
    double residual = t_delta.norm();
    int iter = 0;
    double decay = 1;
    double residualDelta = -1e2;

    while (residual > 1e-5 && iter < 3e2) {

        /* Get Precondition */
        auto t_cal = decay * t_delta;
        twistd Vs = Adjoint(trans_cur) * t_cal;
        Eigen::JacobiSVD<IIWAJacobian> svd(GetJacobian(thetav), Eigen::ComputeFullU | Eigen::ComputeFullV);

        /* Test new Solution is efficient */
        IIWAThetav thetav_new       = thetav +  svd.solve(Vs);
        SE3d       trans_new        = FowardKinematic(thetav_new);
        twistd     t_delta_new      = LogMapSE3Tose3(InverseSE3(trans_new) * trans_desire);
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

    //TODO : 放到 Move
    if(iter >= 1e3)
    {
        std::cout << "Iters:=" << iter << std::endl;
        std::cout << "cur Pos:=\n" <<  trans_cur.block(0, 3, 3, 1)    << std::endl;
        std::cout << "goal Pos:\n" <<  trans_desire.block(0, 3, 3, 1) << std::endl;
        std::cout << "TestGoal:=\n" << trans_desire << "\n\n";
    }
    else
    {
        for(int i = 0; i < 7;++i)
        {
            _thetas[i] = ToStandarAngle(thetav[i]);
        }
    }

    return IIWAThetas{ thetav[0], thetav[1], thetav[2], thetav[3],
                thetav[4], thetav[5], thetav[6]};
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

// TODO: 完善 Move
/// 操作控制类函数
void KUKA_IIWA_MODEL::Move(const IIWAThetas & thetas)
{
    SetThetas(thetas);
}

/// 设置获取类
IIWAThetas KUKA_IIWA_MODEL::GetThetas() const
{
    return _thetas;
}

inline void KUKA_IIWA_MODEL::SetThetas(const IIWAThetas & thetas)
{
    _thetas = thetas;
}
