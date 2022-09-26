#include "motion/trajectory.h"

#include "model/model.h"

#include "component/kinematic_component.h"

namespace GComponent {

/*_______________________________Trajectory Implementation Class_________________________________*/
struct TrajectoryImpl {
/// Constructor
TrajectoryImpl(Model& obj, float time_total);	
/// Fields
Model&				obj;
KinematicComponent& kine_sdk;
SE3f				glb_inv		= SE3f::Identity();
float				time_total	= 0.0f;
};

TrajectoryImpl::TrajectoryImpl(Model& obj, float time_total) :
    obj(obj),
    kine_sdk(*obj.GetComponent<KinematicComponent>(KinematicComponent::type_name.data())),
    time_total(time_total)
{
    glb_inv = obj.getModelMatrixWithoutScale().inverse();
}

/*_______________________________Abstract Trajectory Class_______________________________________*/
Trajectory::Trajectory(Model& obj, float time_total):
    impl_(new TrajectoryImpl(obj, time_total))
{}

Trajectory::~Trajectory()
{}

void Trajectory::SetTimeTotal(float time_total)
{
    impl_->time_total = time_total;
}

float Trajectory::GetTimeTotal() const
{
    return impl_->time_total;
}

Model& Trajectory::GetModel()
{
    return impl_->obj;
}

std::string Trajectory::GetIdentifier() const
{
    return impl_->obj.getName();
}

/*___________________________Joint Space Trajectory_________________________________*/
JTrajectory::JTrajectory(Model& obj, float time_total, _CurveFnc curve_func) :
    Trajectory(obj, time_total),
    fnc_(std::move(curve_func))    
{
    type_ = eJSpace;
}

JTrajectory::~JTrajectory()
{}

JointPairs JTrajectory::operator()(float t_reg)
{    
    return fnc_(t_reg);
}

SE3f JTrajectory::GetInitialPoint() const
{
    SE3f out_mat; impl_->kine_sdk.ForwardKinematic(out_mat, fnc_(0.0f).first);
    return impl_->obj.getModelMatrixWithoutScale() * out_mat;
}

SE3f JTrajectory::GetGoalPoint() const
{
    SE3f out_mat; impl_->kine_sdk.ForwardKinematic(out_mat, fnc_(1.0f).first);
    return impl_->obj.getModelMatrixWithoutScale() * out_mat;
}

Trajectory::_Point3s JTrajectory::GetSampleFromPathFunc(float interval)
{    
    _Point3s samples(static_cast<int>(1 / interval) + 1, _Point3::Zero());
    for (int i = 0; i < samples.size(); ++i) {
        _Point3& cur_pos   = samples[i];
        float    cur_t     = Clamp(i * interval, 0.0f, impl_->time_total);
        SE3f     cur_trans;  impl_->kine_sdk.ForwardKinematic(cur_trans, fnc_(cur_t).first);
        cur_pos  = cur_trans.block(0, 3, 3, 1);
    }
    return samples;
}

Trajectory* JTrajectory::GetPtrCopy() const
{
    return new JTrajectory(*this);
}

/*________________________________Trajectory Class_______________________________________________*/
CTrajectory::CTrajectory(Model& obj, float time_total, PathFunc path_func):
    Trajectory(obj, time_total),
    func_(std::move(path_func))
{
    type_ = eCSpace;
}

CTrajectory::~CTrajectory()		= default;

JointPairs CTrajectory::operator()(float t_reg)
{
    float&              time_total = impl_->time_total;
    SE3f&               glb_inv    = impl_->glb_inv;
    KinematicComponent& kine_sdk   = impl_->kine_sdk;
    const int           kJNum      = kine_sdk.GetJointCount();

    float         t      = Clamp(t_reg / time_total, 0.0f, 1.0f);
    vector<float> cur_xs = kine_sdk.GetJointsPos();
    Twistf        glb_t  = func_(t);
    SE3f          g_ori  = glb_inv * ExpMapping(glb_t),
                  goal   = g_ori * ExpMapping(modify_vec_);
    
    vector<float> out_xs;  kine_sdk.InverseKinematic(out_xs, goal, cur_xs);
    std::transform(out_xs.begin(), out_xs.end(), out_xs.begin(), ToStandarAngle);
                                                      // self decaying
    if (out_xs.empty()) return JointPairs{};                                // solve failed

    modify_vec_ *= 0.995f;
    if (target_opt_ && target_opt_->ConditionCheck(impl_->obj)) {           // modify target       
        out_xs = target_opt_->Optimize(impl_->obj, glb_t, out_xs);
        std::transform(out_xs.begin(), out_xs.end(), out_xs.begin(), ToStandarAngle);
        SE3f new_goal = SE3f::Identity();
        kine_sdk.ForwardKinematic(new_goal, out_xs);
        modify_vec_   = LogMapSE3Tose3(InverseSE3(g_ori) * new_goal);
        modify_vec_   = ClampMaxAbs(DynVec<float>(modify_vec_), 0.35f);
    }

    if (self_opt_ && self_opt_->ConditionCheck(impl_->obj)) {                        // Self Motion
        auto inc = self_opt_->IncVector(impl_->obj, out_xs);
        ZeroProj<float> zero_mat; 
        kine_sdk.ZeroProjection(zero_mat, out_xs);
        auto self_xs = STLUtils::fromDynVecf(inc * zero_mat);
        std::transform(self_xs.begin(), self_xs.end(), out_xs.begin(), out_xs.begin(), std::plus<>{});
        std::transform(out_xs.begin(),  out_xs.end(),  out_xs.begin(), ToStandarAngle);
    }
    
    SE3f          mat_ini;  kine_sdk.ForwardKinematic(mat_ini, cur_xs);
    Jacobi<float> jacobi;   kine_sdk.Jacobian(jacobi, cur_xs);
    Twist<float>  delta_v    = LogMapSE3Tose3(goal * InverseSE3(mat_ini));
    Thetav<float> out_vs_vec = Eigen::JacobiSVD<decltype(jacobi)>(jacobi, Eigen::ComputeFullU | Eigen::ComputeFullV).solve(delta_v);
    Thetas<float> out_vs(out_vs_vec.begin(), out_vs_vec.end());
    
    return {out_xs, out_vs};
}

SE3f CTrajectory::GetInitialPoint() const
{
    return ExpMapping(func_(0.0f));
}

SE3f CTrajectory::GetGoalPoint() const
{
    return ExpMapping(func_(1.0f));
}

Trajectory::_Point3s CTrajectory::GetSampleFromPathFunc(float interval)
{
    _Point3s samples(static_cast<int>(1 / interval) + 1, _Point3::Zero());
    for (int i = 0; i < samples.size(); ++i) {
        float    cur_t     = Clamp(i * interval, 0.0f, 1.0f);
        _Point3& cur_pos   = samples[i];

        cur_pos   = ExpMapping(func_(cur_t)).block(0, 3, 3, 1);
    }
    return samples;
}

Trajectory* CTrajectory::GetPtrCopy() const
{
    return new CTrajectory(*this);
}

PathFunc CTrajectory::GetPathFunction()
{
	return func_;
}

}