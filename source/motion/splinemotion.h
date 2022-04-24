#ifndef GCOMPONENT_SPLINEMOTION_H
#define GCOMPONENT_SPLINEMOTION_H

#include "motion/gmotionbase.h"


namespace GComponent {

// TODO: 抽象出一个大的机器人类替代具体的 KUKA_IIWA_MODEL
class KUKA_IIWA_MODEL;

class SPlineMotion : public GMotionBase
{
public:
    SPlineMotion(const SE3d&, const vector<vec3d>&);
    // Depriciate Constructor Method
    SPlineMotion(const twistd&, const vector<vec3d>&);
    SPlineMotion(const vec3d&, const vector<vec3d>&, const vec3d& ori = vec3d::Zero());

    JointCruveMsgPkg
    GetCurvesFunction(KUKA_IIWA_MODEL*, const double Max_Vel_Limit = 5, const double Max_Acc_Limit = 5);

private:
    SE3d T_goal;
    vector<vec3d> posList_mid;

};

} // namespace GComponent

#endif // GCOMPONENT_SPLINEMOTION_H
