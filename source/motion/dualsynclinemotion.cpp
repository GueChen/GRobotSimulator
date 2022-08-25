#include "motion/dualsynclinemotion.h"

using namespace GComponent;

DualSyncLineMotion::DualSyncLineMotion(const SE3f & mat): DualSyncMotionBase(mat){}

std::pair<PathFunc, PathFunc> GComponent::DualSyncLineMotion::PathFuncImpl(const SE3f& mat_ini_l, const SE3f& mat_end_l, const SE3f& mat_ini_r, const SE3f& mat_end_r)
{
    return std::make_pair(GetScrewLineFunction(mat_ini_l, mat_end_l), 
                          GetScrewLineFunction(mat_ini_r, mat_end_r));
}

float GComponent::DualSyncLineMotion::ExecutionTimeImpl(const SE3f& mat_ini_l, const SE3f& mat_end_l, const SE3f& mat_ini_r, const SE3f& mat_end_r)
{
    float left_dist  = (mat_ini_l.block(0, 3, 3, 1) - mat_end_l.block(0, 3, 3, 1)).norm(),
          right_dist = (mat_ini_r.block(0, 3, 3, 1) - mat_end_r.block(0, 3, 3, 1)).norm();    
    return std::max(left_dist / left_max_vel_, right_dist / right_max_vel_);
}
