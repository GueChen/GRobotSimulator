/**
 *  @file  	dualsynccirclemotion.h
 *  @brief 	execution sync motion with circle trajectory.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 23th, 2022
 **/
#ifndef __DUALSYNCCIRCLE_MOTION_H
#define __DUALSYNCCIRCLE_MOTION_H

#include "motion/gmotionbase.h"

namespace GComponent {

class DualSyncCircleMotion: public DualSyncMotionBase{
public:
	DualSyncCircleMotion(const SE3f& mat, Vec3f waypoint);
	~DualSyncCircleMotion() = default;

protected:    
    virtual 
    std::pair<PathFunc, PathFunc> PathFuncImpl(const SE3f& mat_ini_l, const SE3f& mat_end_l, 
                                               const SE3f& mat_ini_r, const SE3f& mat_end_r) override;
    virtual 
    float                         ExecutionTimeImpl
                                              (const SE3f& mat_ini_l, const SE3f& mat_end_l, 
                                               const SE3f& mat_ini_r, const SE3f& mat_end_r) override;
private:
	Vec3f waypoint_ = Vec3f::Zero();

	
};

} // !namespace GComponent

#endif // !__DUALSYNCCIRCLE_MOTION_H
