#include "motion/splinemotion.h"

namespace GComponent {

SPlineMotion::SPlineMotion(const SE3f& mat, const vector<Vec3f>& poses, SplineType type):
    CMotionBase(mat), mid_poses_(poses), type_(type)
{}

PathFunc SPlineMotion::PathFuncImpl(const SE3f & mat_ini, const SE3f & mat_end)
{
    Twistf t_ini = LogMapSE3Tose3(mat_ini), 
           t_end = LogMapSE3Tose3(mat_end);
    switch (type_) {
    case SplineType::Cubic:     
        return GetCubicSplineFunction(t_ini, t_end, mid_poses_);
    case SplineType::Bezier:    
        return GetBezierSplineFunctoin(t_ini, t_end, mid_poses_);
    case SplineType::BezierInter:
        return GetBezierInterSplineFunction(t_ini, t_end, mid_poses_);
    case SplineType::BSpline: 
        return GetBSplineInterFunction(t_ini, t_end, mid_poses_, false);
    case SplineType::BSplineInter:
        return GetBSplineInterFunction(t_ini, t_end, mid_poses_, true);
    default:
        assert(false && "Never Happen, Undefined Case");
    }
    
}

float SPlineMotion::ExecutionTimeImpl(const SE3f& mat_ini, const SE3f& mat_end)
{
    Vec3f pos_ini     = mat_ini.block(0, 3, 3, 1),
          pos_goal    = mat_end.block(0, 3, 3, 1);
    float lin_dist    = 0.0f;
    lin_dist += (pos_ini - mid_poses_.front()).norm();
    lin_dist += (pos_goal - mid_poses_.back()).norm();
    for (auto it = mid_poses_.begin(), end = std::prev(mid_poses_.end()); 
        it != end;) {
        auto it_next = std::next(it);
        lin_dist += (*it - *it_next).norm();
        it = it_next;
    }
    float ang_dist = (LogMapSO3Toso3(mat_ini.block(0, 0, 3, 3).inverse() * goal_.block(0, 0, 3, 3))).norm();
    return std::max(lin_dist / max_vel_, ang_dist / max_ang_vel_);
}

} // namespace GComponent
