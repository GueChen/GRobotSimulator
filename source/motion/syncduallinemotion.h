#ifndef SYNCDUALLINEMOTION_H
#define SYNCDUALLINEMOTION_H

#include "motion/gmotionbase.h"

namespace GComponent{

class KUKA_IIWA_MODEL;

class SyncDualLineMotion : public GComponent::GMotionBase
{
public:
    explicit SyncDualLineMotion(const SE3d &);

    DualJointMsgPkg
    GetCurvesFunction(KUKA_IIWA_MODEL*, KUKA_IIWA_MODEL*,
                      const SE3d&     , const SE3d&     ,
                      const double MaxVelLimitLeft = 0.1, const double MaxVelLimitRight = 0.1,
                      const double MaxAccLimitLeft = 0.5, const double MaxAccLimitRight = 0.5);
    tuple<SE3d, SE3d> GetTGoalTransforms(KUKA_IIWA_MODEL*, KUKA_IIWA_MODEL*, const SE3d &, const SE3d &);
private:
    SE3d T_goal_;
};
}
#endif // SYNCDUALLINEMOTION_H
