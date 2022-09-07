#include "motion/trajectory.h"

#include "model/model.h"

#include "component/kinematic_component.h"

namespace GComponent {

struct TrajectoryImpl {
/// Constructor
TrajectoryImpl(Model& obj, float time_total):
	obj       (obj),
	kine_sdk  (*obj.GetComponent<KinematicComponent>(KinematicComponent::type_name.data())),
    time_total(time_total)
{
	glb_inv = obj.getModelMatrixWithoutScale().inverse();
}
/// Fields
Model&				obj;
KinematicComponent& kine_sdk;
SE3f				glb_inv		= SE3f::Identity();
float				time_total	= 0.0f;
};

Trajectory::Trajectory(Model& obj, PathFunc path_func, float time_total):
	impl_(new TrajectoryImpl(obj, time_total)),
    path_func_(path_func)
{}

Trajectory::~Trajectory()		= default;

JointPairs Trajectory::operator()(float t_reg)
{
    float&              time_total = impl_->time_total;
    SE3f&               glb_inv    = impl_->glb_inv;
    KinematicComponent& kine_sdk   = impl_->kine_sdk;
    const int           kJNum      = kine_sdk.GetJointCount();

    float         t      = Clamp(t_reg / time_total, 0.0f, 1.0f);
    vector<float> cur_xs = kine_sdk.GetJointsPos();
    Twistf        glb_t  = path_func_(t);
    SE3f          goal   = glb_inv * ExpMapping(glb_t);
    vector<float> out_xs;  kine_sdk.InverseKinematic(out_xs, goal, cur_xs);
    std::transform(out_xs.begin(), out_xs.end(), out_xs.begin(), ToStandarAngle);
    if (out_xs.empty()) return JointPairs{};                         // Solve Failed

    if (target_opt_ && target_opt_->ConditionCheck(impl_->obj)) {           // Modify Target
        glb_t = target_opt_->Optimize(impl_->obj, glb_t, out_xs);
        goal  = glb_inv * ExpMapping(glb_t);
        kine_sdk.InverseKinematic(out_xs, goal, cur_xs);
        std::transform(out_xs.begin(), out_xs.end(), out_xs.begin(), ToStandarAngle);
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
    Twist<float>  delta_v = LogMapSE3Tose3(goal * InverseSE3(mat_ini));
    Thetav<float> out_vs_vec = Eigen::JacobiSVD<decltype(jacobi)>(jacobi, Eigen::ComputeFullU | Eigen::ComputeFullV).solve(delta_v);
    Thetas<float> out_vs(out_vs_vec.begin(), out_vs_vec.end());
    
    return {out_xs, out_vs};
}

void Trajectory::SetTimeTotal(float time_total)
{
    impl_->time_total = time_total;
}

float Trajectory::GetTimeTotal() const
{
    return impl_->time_total;
}

std::vector<std::vector<float>> Trajectory::GetSampleFromPathFunc(float interval)
{
    std::vector<std::vector<float>> samples(static_cast<int>(1 / interval) + 1, vector<float>(3));
    for (int i = 0; i < samples.size(); ++i) {
        float  cur_t     = Clamp(i * interval, 0.0f, 1.0f);
        Twistf cur_point = path_func_(cur_t);
        Vec3   cur_pos   = ExpMapping(cur_point).block(0, 3, 3, 1);
        std::transform(cur_pos.begin(), cur_pos.end(), samples[i].begin(),
            [](auto&& val) {return val; });
    }
    return samples;
}

Model& Trajectory::GetModel()
{
    return impl_->obj;
}

PathFunc Trajectory::GetPathFunction()
{
	return path_func_;
}

}