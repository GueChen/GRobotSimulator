#ifndef GCOMPONENT_SPLINEMOTION_H
#define GCOMPONENT_SPLINEMOTION_H

#include "motion/gmotionbase.h"


namespace GComponent {

class SPlineMotion : public CMotionBase
{
public:
    SPlineMotion(const SE3f&, const vector<Vec3f>&);
    ~SPlineMotion() = default;

protected:
    virtual PathFunc PathFuncImpl(const SE3f& mat_ini, const SE3f& mat_end)      override;
    virtual float    ExecutionTimeImpl(const SE3f& mat_ini, const SE3f& mat_end) override;

private:
    vector<Vec3f> mid_poses_;
    
};

} // namespace GComponent

#endif // GCOMPONENT_SPLINEMOTION_H
