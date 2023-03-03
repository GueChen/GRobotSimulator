#include "motion/keepermotion.h"

namespace GComponent {

GComponent::KeeperMotion::KeeperMotion(const SE3f& goal, float execution_time):
    CMotionBase(goal)
{
    time_total_ = execution_time;
}

PathFunc GComponent::KeeperMotion::PathFuncImpl(const SE3f& mat_ini, const SE3f& mat_end)
{
    return [goal = goal_](float t) {return LogMapSE3Tose3(goal); };
}

float GComponent::KeeperMotion::ExecutionTimeImpl(const SE3f& mat_ini, const SE3f& mat_end)
{
    return time_total_;
}
}