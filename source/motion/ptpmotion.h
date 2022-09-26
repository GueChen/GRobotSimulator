#ifndef PTPMOTION_H
#define PTPMOTION_H

#include "motion/gmotionbase.h"

namespace GComponent {

class PTPMotion : public JMotionBase
{
public:
    explicit PTPMotion(const vector<float>&, InterpolationEnum type = InterpolationEnum::Trapezoid);
    ~PTPMotion() = default;

    virtual JTrajectory operator()(Model* robot) override;
       
private:
    vector<float> jgoals_;
};

}

#endif // PTPMOTION_H
