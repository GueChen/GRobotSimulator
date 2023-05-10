#include "ptpmotion.h"

#include "component/kinematic_component.h"
#include "component/joint_group_component.h"

#include <ranges>

using namespace GComponent;

PTPMotion::PTPMotion(const vector<float>& thetas_desire, InterpolationEnum type):
    jgoals_(thetas_desire)
{}

JTrajectory GComponent::PTPMotion::operator()(Model* robot)
{
    struct QuadParam {
        int     sgn;        // the positive and negative sign
        float   t_rise;     // acc/dec time
        float   delta;      // delta value
    };

    // Get interface to posses datas
    KinematicComponent&  kinematic_sdk  = *robot->GetComponent<KinematicComponent>();
    JointGroupComponent& joints_sdk     = *robot->GetComponent<JointGroupComponent>();    
    vector<float>        j_vals         = joints_sdk.GetPositions();
    const float          kMaxAccTime    = max_vel_ / max_acc_;
    const uint32_t       kJNum          = joints_sdk.GetJointsSize();

    // Caculate max delta joint val
    vector<float> j_delta(kJNum);
    std::transform(jgoals_.begin(), jgoals_.end(), j_vals.begin(),
                   j_delta.begin(),
                   std::minus<>{}
    );
    float max_delta  = std::abs(*std::ranges::max_element(j_delta,
        [](auto& num1, auto& num2) { return std::abs(num1) < std::abs(num2); }
    ));

    // Caculate total time of sync j motion
    float time_total = 0.0f;
    if (max_acc_ >= 1e-15)
    if (float max_acc_dist = max_vel_ * max_vel_ / max_acc_;
        max_acc_dist >= max_delta){
        time_total = 2.0 * std::sqrt(max_delta / max_acc_);
    }
    else{
        time_total = max_delta / max_vel_ + kMaxAccTime;
    }
    time_total_ = time_total;

    // Caculate acc params
    vector<QuadParam> params(kJNum);
    std::transform(j_delta.begin(), j_delta.end(), params.begin(),
        // solve equation: A * x * x + B * x + C = 0
        [A = max_acc_, B = -max_acc_ * time_total](float C)->QuadParam{
            int   sgn    = C < 0 ? -1 : 1;
            float t_rise = (-B - std::sqrt(B * B - 4 * A * std::abs(C))) / (2 * A),
                  delta  = C;
            return {sgn, t_rise, delta};}
    );

    // all deserve, get the function
    auto traj_func = [acc = max_acc_, params, time_total, kJNum, j_vals]
    (float t)->JointPairs {
        JointPairs ret;
        ret.first.resize(kJNum);
        ret.second.resize(kJNum);
        for (int i = 0; i < kJNum; ++i) {
            auto& [sgn, t_rise, delta] = params[i];
            float& x                   = ret.first[i],
                 & v                   = ret.second[i];
            x = j_vals[i];

            if (t <= t_rise){
                v =  sgn * acc * t;
                x += 0.5 * v * t;
            }
            else if (t <= time_total - t_rise){
                v =  sgn * acc * t_rise;
                x += v * (t - 0.5 * t_rise);
            }
            else if (t <= time_total - 1e-5){
                v =  sgn * acc * (time_total - t);
                x += delta - 0.5 * v * (time_total - t);
            }
            else{
                v =  0.0f;
                x += delta;
            }            
        }        
        return ret;      
    };

    return JTrajectory(*robot, time_total, traj_func);
}
