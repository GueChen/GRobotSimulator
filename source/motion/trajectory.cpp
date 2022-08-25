#include "motion/trajectory.h"

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
    SE3f          goal   = glb_inv * ExpMapping(path_func_(t));
    vector<float> out_xs; kine_sdk.InverseKinematic(out_xs, goal, cur_xs);
    std::transform(out_xs.begin(), out_xs.end(), out_xs.begin(), ToStandarAngle);

    if (out_xs.empty()) return JointPairs{};    // Solve Failed

    SE3f          mat_ini;  kine_sdk.ForwardKinematic(mat_ini, cur_xs);
    Jacobi<float> jacobi;   kine_sdk.Jacobian(jacobi, cur_xs);
    Twist<float>  delta_v = LogMapSE3Tose3(goal * InverseSE3(mat_ini));
    Thetav<float> out_vs  = Eigen::JacobiSVD<decltype(jacobi)>(jacobi, Eigen::ComputeFullU | Eigen::ComputeFullV).solve(delta_v);

    JointPairs ret(kJNum);
    for (int i = 0; i < kJNum; ++i) {
        JointPair& joint = ret[i];
        joint.first = out_xs[i];
        joint.second = out_vs[i];
    }
    return ret;
}

void Trajectory::SetTimeTotal(float time_total)
{
    impl_->time_total = time_total;
}

float Trajectory::GetTimeTotal() const
{
    return impl_->time_total;
}

std::vector<std::vector<float>> Trajectory::SampleFromPathFunc(float interval)
{
    std::vector<std::vector<float>> samples(static_cast<int>(1 / interval) + 1, vector<float>(3));
    for (int i = 0; i < samples.size(); ++i) {
        float  cur_t     = Clamp(i * interval, 0.0f, 1.0f);
        Twistf cur_point = path_func_(cur_t);
        std::transform(cur_point.begin(), cur_point.begin() + 3, samples[i].begin(), 
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