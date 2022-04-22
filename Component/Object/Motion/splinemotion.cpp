#include "splinemotion.h"
#include "Component/Object/Robot/kuka_iiwa_model.h"

namespace GComponent {

SPlineMotion::SPlineMotion(const SE3d& T, const vector<vec3d>& pList):
    T_goal(T), posList_mid(pList)

{}

JointCruveMsgPkg SPlineMotion::GetCurvesFunction(KUKA_IIWA_MODEL * robot, const double Max_Vel_Limit, const double Max_Acc_Limit)
{
    SE3d T_ini = robot->ForwardKinematic();
    vec3d p_ini = T_ini.block(0, 3, 3, 1);
    vec3d p_end = T_goal.block(0, 3, 3, 1);
    const size_t JointNum = robot->GetThetas().size();

    auto SPlineFunc = GetCubicSplineFunction(LogMapSE3Tose3(T_ini), LogMapSE3Tose3(T_goal), posList_mid);
    double scaler = 0.0;
    scaler += (p_ini - posList_mid.front()).norm();
    scaler += (p_end - posList_mid.back()).norm();
    for(auto it = posList_mid.begin(); it != posList_mid.end() - 1; ++it)
    {
        scaler += (*it - *(it + 1)).norm();
    }
    double tot = scaler / Max_Vel_Limit;

    /* 没有任何连续性考虑的测试 */
    auto PositionFunction = [robot, Max_Vel_Limit, SPlineFunc, tot](double t){
        double scalered_t  = Clamp(t / tot, 0.0, 1.0);
        twistd twist_cur   = SPlineFunc(scalered_t);
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

} // namespace GComponent
