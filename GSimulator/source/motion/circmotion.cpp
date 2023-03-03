#include "circmotion.h"

using namespace GComponent;

CircMotion::CircMotion(const SE3f& mat, const Vec3f& pos):
    CMotionBase(mat), pos_mid_(pos)
{}

PathFunc GComponent::CircMotion::PathFuncImpl(const SE3f & mat_ini, const SE3f & mat_end)
{
    return GetCircleFunction(mat_ini, mat_end, pos_mid_);
}

float GComponent::CircMotion::ExecutionTimeImpl(const SE3f& mat_ini, const SE3f& mat_end)
{
    Vec3f pos_ini    = mat_ini.block(0, 3, 3, 1),
          pos_goal   = mat_end.block(0, 3, 3, 1);
    float radius     = GetRadiusOfCircle(pos_ini, pos_mid_, pos_goal),
          angle      = GetArcDeltaOfCircle(pos_ini, pos_mid_, pos_goal);  
    float lin_dist   = angle * radius;    
    float ang_dist   = (LogMapSO3Toso3(mat_ini.block(0, 0, 3, 3).inverse() * goal_.block(0, 0, 3, 3))).norm();   
    return std::max(lin_dist / max_vel_, ang_dist / max_ang_vel_);
}

