#ifndef SYNCDUALLINEMOTION_H
#define SYNCDUALLINEMOTION_H

#include "motion/gmotionbase.h"

namespace GComponent{

class DualSyncLineMotion : public DualSyncMotionBase
{
public:
    explicit DualSyncLineMotion(const SE3f &);
       
protected:    
    virtual 
    std::pair<PathFunc, PathFunc> PathFuncImpl(const SE3f& mat_ini_l, const SE3f& mat_end_l, 
                                               const SE3f& mat_ini_r, const SE3f& mat_end_r) override;
    virtual 
    float                         ExecutionTimeImpl
                                              (const SE3f& mat_ini_l, const SE3f& mat_end_l, 
                                               const SE3f& mat_ini_r, const SE3f& mat_end_r) override;

};
}
#endif // SYNCDUALLINEMOTION_H
