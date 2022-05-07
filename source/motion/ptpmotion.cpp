#include "ptpmotion.h"

#include "model/robot/kuka_iiwa_model.h"


using namespace GComponent;

PTPMotion::PTPMotion(const vector<double>& thetas_desire, InterpolationEnum type):
    thetas_goal(thetas_desire)
{}

JointCruveMsgPkg PTPMotion::GetCurvesFunction(KUKA_IIWA_MODEL * robot, const double Max_Vel_Limit, const double Max_Acc_Limit)
{
    /* 获取机器人输入 */
    auto thetas_ini = robot->GetThetas();
    if(thetas_ini.size() != thetas_goal.size())
    {
        throw(std::exception("Wrong Thetas Number"));
    }

    /* 设置限制条件 */
    const double Max_Acc_Time  = Max_Vel_Limit / Max_Acc_Limit;
    const int    JointNum = thetas_ini.size();

    /* 计算差角 */
    decltype (thetas_ini) thetas_delta;
    std::transform(std::execution::par,
                   thetas_goal.begin(),
                   thetas_goal.end(),
                   thetas_ini.begin(),
                   thetas_delta.begin(),
                   [](auto & goal, auto & ini){
                        return goal- ini;
                   }
    );

    /* 计算耗时 */
    auto max_delta = std::abs(*std::max_element(
                thetas_delta.begin(),
                thetas_delta.end(),
                [](auto & num1, auto & num2){
                    return std::abs(num1) < std::abs(num2);
                }
    ));
    std::cout << "max_delta:= "<< max_delta << std::endl;
    double time_total = 0;
    // 判断计算情况是否全加速
    if(double MaxAccDistance = std::pow(Max_Vel_Limit, 2) / Max_Acc_Limit;
              MaxAccDistance >= max_delta)
    {
        time_total = 2.0 * std::sqrt(max_delta / Max_Acc_Limit);
    }
    else
    {
        time_total = max_delta/ Max_Vel_Limit + Max_Acc_Time;
    }

    array<tuple<int, double, double>, JointNum> msg_list;
    std::transform(thetas_delta.begin(),
                   thetas_delta.end(),
                   msg_list.begin(),
                   [&GetThis = Max_Acc_Limit, B = -Max_Acc_Limit*time_total](auto & C){
                        return std::make_tuple(
                                    C < 0? -1:1,
                                    (-B-std::sqrt(std::pow(B, 2) - 4 * GetThis * std::abs(C))) / (2 * GetThis),
                                    C);
                   }
    );

    /* 计算速度 */
    auto VelocityFunction =
            [acc = Max_Acc_Limit, control_list = msg_list, tot = time_total, N = JointNum]
            (double t){
         vector<double> ret_Vels(N);
         std::transform(std::execution::par,
                        control_list.begin(), control_list.end(),
                        ret_Vels.begin(),
                        [t, acc, &tot](auto & accMsg){
                            auto & [sgn, accTime, _] = accMsg;
                            if(t <= accTime)             // 加速段
                            {
                                return sgn * acc * t;
                            }
                            else if(t <= tot - accTime)  // 匀速段
                            {
                                return sgn * acc * accTime;
                            }
                            else if(t <= tot - 1e-5)     // 减速段
                            {
                                return sgn * acc * (tot - t);
                            }
                            else                         // 结束段
                            {
                                return 0.0;
                            }
                        }
         );
         return ret_Vels;
    };

    /* 计算位置 */
    auto PositionFunction =
            [acc = Max_Acc_Limit, control_list = msg_list, tot = time_total, N = JointNum,
             thetas_ini]
            (double t){
            vector<double> ret_Pos(N);
            std::transform(std::execution::par,
                           control_list.begin(), control_list.end(),
                           thetas_ini.begin(),
                           ret_Pos.begin(),
                           [t, acc, &tot](auto&& accMsg, auto&& theta_ini){
                                auto & [sgn, accTime, theta_delta] = accMsg;
                                double delta = 0.0;
                                if(t <= accTime)             // 加速段
                                {
                                    delta = 0.5 * sgn *  acc * std::pow(t, 2);
                                }
                                else if(t <= tot - accTime)  // 匀速段
                                {
                                    delta = sgn * acc * accTime * (t - 0.5 * accTime);
                                }
                                else if(t <= tot - 1e-5)     // 减速段
                                {
                                    delta = theta_delta - 0.5 * sgn * acc * std::pow(tot - t, 2);
                                }
                                else                         // 结束段
                                {
                                    delta = theta_delta;
                                }
                                return theta_ini + delta;
                           }
            );
            return ret_Pos;
    };

    return {time_total, PositionFunction, VelocityFunction};
}
