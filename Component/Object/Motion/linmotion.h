#ifndef LINMOTION_H
#define LINMOTION_H

#include <Component/Object/gmotionbase.h>


namespace GComponent {

// TODO: 抽象出一个大的机器人类替代具体的 KUKA_IIWA_MODEL
class KUKA_IIWA_MODEL;

class LinMotion : public GMotionBase
{
public:
    explicit LinMotion(const SE3d &);
    // Depriciate Constructor Method
    explicit LinMotion(const twistd&);
    explicit LinMotion(const vec3d &, const vec3d& = vec3d::Zero());

    JointCruveMsgPkg
    GetCurvesFunction(KUKA_IIWA_MODEL*, const double Max_Vel_Limit = 5, const double Max_Acc_Limit = 5);

private:
    SE3d T_goal;

};

}

#endif // LINMOTION_H
