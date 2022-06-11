#ifndef PTPMOTION_H
#define PTPMOTION_H

#include "motion/gmotionbase.h"


namespace GComponent {

// TODO: 抽象出一个大的机器人类替代具体的 KUKA_IIWA_MODEL
class KUKA_IIWA_MODEL;

class PTPMotion : public GMotionBase
{
public:
    explicit PTPMotion(const vector<double>&, InterpolationEnum type = InterpolationEnum::Trapezoid);
    // Depriciate Constructor Method
    explicit PTPMotion(const SE3d&);
    explicit PTPMotion(const Vec3d &);

    JointCruveMsgPkg
    GetCurvesFunction(KUKA_IIWA_MODEL*, const double Max_Vel_Limit = 5, const double Max_Acc_Limit = 5);

private:
    vector<double> thetas_goal;

};

}

#endif // PTPMOTION_H
