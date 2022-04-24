#ifndef CIRCMOTION_H
#define CIRCMOTION_H

#include "motion/gmotionbase.h"


namespace GComponent {

// TODO: 抽象出一个大的机器人类替代具体的 KUKA_IIWA_MODEL
class KUKA_IIWA_MODEL;

class CircMotion : public GMotionBase
{
public:
    CircMotion(const SE3d&, const vec3d&);
    // Depriciate Constructor Method
    CircMotion(const twistd&, const vec3d&);
    CircMotion(const vec3d&,  const vec3d&, const vec3d& = vec3d::Zero());

    JointCruveMsgPkg
    GetCurvesFunction(KUKA_IIWA_MODEL*, double Max_Vel_Limit = 5, double Max_Acc_Limit = 5);

private:
    vec3d pos_mid;
    SE3d  T_goal;

};

}

#endif // CIRCMOTION_H
