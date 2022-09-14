#ifndef __LINE_MOTION_H
#define __LINE_MOTION_H

#include "motion/gmotionbase.h"

namespace GComponent {

using BallObst = pair<Vec3d, double>;

class LineMotion : public CMotionBase
{
public:
    explicit LineMotion(const SE3f& goal);
    ~LineMotion() = default;

protected:      
    // 通过 CMotionBase 继承
    virtual PathFunc PathFuncImpl(const SE3f& mat_ini, const SE3f& mat_end)         override;
    virtual float    ExecutionTimeImpl(const SE3f& mat_ini, const SE3f& mat_end)    override;

};

}

#endif // __LINE_MOTION_H
