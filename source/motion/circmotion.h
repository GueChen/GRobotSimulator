#ifndef CIRCMOTION_H
#define CIRCMOTION_H

#include "motion/gmotionbase.h"


namespace GComponent {

class CircMotion : public CMotionBase
{
public:
    CircMotion(const SE3f&, const Vec3f&);
    ~CircMotion() = default;

protected:
    virtual PathFunc PathFuncImpl(const SE3f& mat_ini, const SE3f& mat_end)      override;
    virtual float    ExecutionTimeImpl(const SE3f& mat_ini, const SE3f& mat_end) override;

private:
    Vec3f pos_mid_;
};

}

#endif // CIRCMOTION_H
