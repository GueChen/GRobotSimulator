#include "motion/linmotion.h"

using namespace GComponent;

LineMotion::LineMotion(const SE3f&  mat):
    CMotionBase(mat)    
{}

PathFunc GComponent::LineMotion::PathFuncImpl(const SE3f& mat_ini, const SE3f& mat_end)
{
    return GetScrewLineFunction(mat_ini, goal_);
}

float GComponent::LineMotion::ExecutionTimeImpl(const SE3f& mat_ini, const SE3f& mat_end)
{
    float lin_dist = (mat_ini.block(0, 3, 3, 1) - mat_end.block(0, 3, 3, 1)).norm();
    float ang_dist = (LogMapSO3Toso3(mat_ini.block(0, 0, 3, 3).inverse() * mat_end.block(0, 0, 3, 3))).norm();    
    return std::max(lin_dist / max_vel_, ang_dist / max_ang_vel_);
}

