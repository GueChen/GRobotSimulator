#include "dualsynccirclemotion.h"

namespace GComponent {
GComponent::DualSyncCircleMotion::DualSyncCircleMotion(const SE3f& mat, Vec3f waypoint):
	DualSyncMotionBase(mat),
	waypoint_(waypoint)
{}

std::pair<PathFunc, PathFunc> DualSyncCircleMotion::PathFuncImpl(const SE3f & mat_ini_l, const SE3f & mat_end_l, const SE3f & mat_ini_r, const SE3f & mat_end_r)
{
    SO3<float> R              = center_goal_.block(0, 0, 3, 3);
	Vec3f      l_mid  = R * left_trans_.block(0, 3, 3, 1)  + waypoint_,
               r_mid = R * right_trans_.block(0, 3, 3, 1) + waypoint_;
    return std::make_pair(GetCircleFunction(mat_ini_l, mat_end_l, l_mid),
                          GetCircleFunction(mat_ini_r, mat_end_r, r_mid));
}

float DualSyncCircleMotion::ExecutionTimeImpl(const SE3f& mat_ini_l, const SE3f& mat_end_l, const SE3f& mat_ini_r, const SE3f& mat_end_r)
{
     Vec3f     pos_ini_l   = mat_ini_l.block(0, 3, 3, 1), 
               pos_ini_r   = mat_ini_r.block(0, 3, 3, 1),
               pos_goal_l  = mat_end_l.block(0, 3, 3, 1),
               pos_goal_r  = mat_end_r.block(0, 3, 3, 1);
    SO3<float> R           = center_goal_.block(0, 0, 3, 3);
	Vec3f      l_mid       = R * left_trans_.block(0, 3, 3, 1)  + waypoint_,
               r_mid       = R * right_trans_.block(0, 3, 3, 1) + waypoint_;
    float      radius_l    = GetRadiusOfCircle(pos_ini_l, l_mid, pos_goal_l),
               radius_r    = GetRadiusOfCircle(pos_ini_r, r_mid, pos_goal_r),
               angle_l     = GetArcDeltaOfCircle(pos_ini_l, l_mid, pos_goal_l),
               angle_r     = GetArcDeltaOfCircle(pos_ini_r, r_mid, pos_goal_r);
    float      scale_l     = angle_l * radius_l, 
               scale_r     = angle_r * radius_r;    
    return std::max(scale_l / left_max_vel_, scale_r / right_max_vel_);
}

}// !namespace GComponent

