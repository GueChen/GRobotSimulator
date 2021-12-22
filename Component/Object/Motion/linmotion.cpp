#include "linmotion.h"
#include "Component/Object/Robot/kuka_iiwa_model.h"

using namespace GComponent;

LinMotion::LinMotion(const SE3d & T):
    T_goal(T)
{};


JointCruveMsgPkg LinMotion::GetCurvesFunction(KUKA_IIWA_MODEL * robot, const double Max_Vel_Limit, const double Max_Acc_Limit)
{

    SE3d T_ini = robot->FowardKinematic();
    const size_t JointNum = robot->GetThetas().size();

    auto LineFunc = GetScrewLineFunction(T_ini, T_goal);
    double scaler = (T_ini.block(0, 3, 3, 1) - T_goal.block(0, 3, 3, 1)).norm();
    double tot = scaler / Max_Vel_Limit;

    /* 没有任何连续性考虑的测试 */
    auto PositionFunction = [robot, Max_Vel_Limit, LineFunc, tot](double t){
        double scalered_t  = Clampd(t / tot, 0.0, 1.0);
        twistd twist_cur   = LineFunc(scalered_t);
        IIWAThetas ret_Pos = robot->BackKinematic(twist_cur);
        std::for_each(ret_Pos.begin(), ret_Pos.end(),[](auto & num)
        {
            num = ToStandarAngle(num);
            num = RadiusToDegree(num);
        });
        return vector<double>(ret_Pos.cbegin(), ret_Pos.cend());
    };

    // FIXME: 待实装修改
    auto VelocityFunction = [robot, Max_Vel_Limit, tot, JointNum, PositionFunction](double t){
        if(t <= tot || t >= tot) return vector<double>(7, 0.0);
        twistd v_thetav;
        for(int i = 0; i < 6; ++i)
        {
            v_thetav[i] = Max_Vel_Limit;
        }

        return vector<double>(7, Max_Vel_Limit);
    };
    return {tot, PositionFunction, VelocityFunction};
}
