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

#ifdef _OLD_INTERFACE
    void addObstacle(const string&, BallObst&&);
    void updateObstacle(const string&, Vec3d);
    void deleteObstacle(const string&);

    JointCruveMsgPkg
        GetCurvesFunction(KUKA_IIWA_MODEL*, const double Max_Vel_Limit = 5, const double Max_Acc_Limit = 5);

    JointCruveMsgPkg
        GetAvoidObstCurvesFunction(KUKA_IIWA_MODEL*, double Padding = 0.02, const double Max_Vel_Limit = 5, const double Max_Acc_Limit = 5);

    JointCruveMsgPkg
        GetAvoidLimitationCurvesFunction(KUKA_IIWA_MODEL*, const double Max_Vel_Limit = 5, const double Max_Acc_Limit = 5);
#endif // _OLD_INTERFACE

#ifdef _OLD_INTERFACE 
    /* 可抽象出一个球类表述 */
    // map<string, BallObst> obst_Dict;  
#endif // _OLD_INTERFACE
};

}

#endif // __LINE_MOTION_H
