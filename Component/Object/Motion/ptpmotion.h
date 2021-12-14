#ifndef PTPMOTION_H
#define PTPMOTION_H

#include <vector>
#include <functional>
#include <GComponent/GTransform.hpp>
#include "Component/Object/gmotionbase.h"


namespace GComponent {

using std::vector;
using std::function;

// TODO: 抽象出一个大的机器人类替代具体的 KUKA_IIWA_MODEL
class KUKA_IIWA_MODEL;

class PTPMotion : public GMotionBase
{
public:
    explicit PTPMotion(const vector<double>&, InterpolationEnum type = InterpolationEnum::Trapezoid);
    // Depriciate Constructor Method
    explicit PTPMotion(const SE3d&);
    explicit PTPMotion(const vec3d &);

    function<vector<double>(double)> GetCurvesFunction(KUKA_IIWA_MODEL*);
private:
    vector<double> thetas_goal;
};

}
#endif // PTPMOTION_H
