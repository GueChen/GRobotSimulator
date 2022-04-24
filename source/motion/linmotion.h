#ifndef LINMOTION_H
#define LINMOTION_H

#include "motion/gmotionbase.h"


namespace GComponent {

using BallObst = pair<vec3d, double>;


// TODO: 抽象出一个大的机器人类替代具体的 KUKA_IIWA_MODEL
class KUKA_IIWA_MODEL;

class LinMotion : public GMotionBase
{
public:
    explicit LinMotion(const SE3d &);
    // Depriciate Constructor Method
    explicit LinMotion(const twistd&);
    explicit LinMotion(const vec3d &, const vec3d& = vec3d::Zero());

    // TODO: 该归入一个抽象父类中
    void addObstacle(const string&, BallObst &&);
    void updateObstacle(const string&, vec3d);
    void deleteObstacle(const string&);

    JointCruveMsgPkg
    GetCurvesFunction(KUKA_IIWA_MODEL*, const double Max_Vel_Limit = 5, const double Max_Acc_Limit = 5);

    JointCruveMsgPkg
    GetAvoidObstCurvesFunction(KUKA_IIWA_MODEL*, double Padding = 0.02, const double Max_Vel_Limit = 5, const double Max_Acc_Limit = 5);

    JointCruveMsgPkg
    GetAvoidLimitationCurvesFunction(KUKA_IIWA_MODEL*, const double Max_Vel_Limit = 5, const double Max_Acc_Limit = 5);
private:
    SE3d T_goal;
    // TODO: 考虑兼容除避碰球以外的障碍物抽象
    /* 可抽象出一个球类表述 */
    map<string, BallObst> obst_Dict;
};

}

#endif // LINMOTION_H
