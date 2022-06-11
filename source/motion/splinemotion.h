#ifndef GCOMPONENT_SPLINEMOTION_H
#define GCOMPONENT_SPLINEMOTION_H

#include "motion/gmotionbase.h"


namespace GComponent {

// TODO: 抽象出一个大的机器人类替代具体的 KUKA_IIWA_MODEL
class KUKA_IIWA_MODEL;

class SPlineMotion : public GMotionBase
{
public:
    SPlineMotion(const SE3d&, const vector<Vec3d>&);
    // Depriciate Constructor Method
    SPlineMotion(const Twistd&, const vector<Vec3d>&);
    SPlineMotion(const Vec3d&, const vector<Vec3d>&, const Vec3d& ori = Vec3d::Zero());

    JointCruveMsgPkg
    GetCurvesFunction(KUKA_IIWA_MODEL*, const double Max_Vel_Limit = 5, const double Max_Acc_Limit = 5);

private:
    SE3d T_goal;
    vector<Vec3d> posList_mid;

};

} // namespace GComponent

#endif // GCOMPONENT_SPLINEMOTION_H
