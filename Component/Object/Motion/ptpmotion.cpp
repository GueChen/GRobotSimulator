#include <algorithm>
#include <execution>
#include <ranges>
#include <string>
#include <iostream>

#include "ptpmotion.h"
#include "Component/Object/Robot/kuka_iiwa_model.h"


using namespace GComponent;

PTPMotion::PTPMotion(const vector<double>& thetas_desire, InterpolationEnum type):
    thetas_goal(thetas_desire)
{}

function<vector<double>(double)> PTPMotion::GetCurvesFunction(KUKA_IIWA_MODEL * robot)
{
    /* 获取机器人输入 */
    auto thetas_ini = robot->GetThetas();
    if(thetas_ini.size() != thetas_goal.size())
    {
        throw(std::exception("Wrong Thetas Number"));
    }

    /* 设置限制条件 */
    const double Max_Vel_Limit = 5;
    const double Max_Acc_Limit = 5;
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
                thetas_ini.end(),
                [](auto & num1, auto &num2){
                    return std::abs(num1) < std::abs(num2);
                }
    ));

    double time_total = 0;
    std::cout << "Max Delta =" << max_delta << std::endl;
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

    array<pair<double,int>, JointNum> acc_msg_list;

    std::transform(thetas_delta.begin(),
                   thetas_delta.end(),
                   acc_msg_list.begin(),
                   [&A = Max_Acc_Limit, B = -Max_Acc_Limit*time_total](auto & C){
                        std::cout << "B value: = " << B << std::endl;
                        return std::make_pair((-B-std::sqrt(std::pow(B, 2) - 4 * A * std::abs(C))) / (2 * A), C < 0? -1:1);
                   }
    );

    for(auto& val: acc_msg_list)
    {
        std::cout << val.first << " ";
    }
    std::cout << std::endl;
    std::cout << "tot = " << time_total;
    auto VelocityFunction =
            [acc = Max_Acc_Limit, control_list = acc_msg_list, tot = time_total, N = JointNum]
            (double t){
         vector<double> ret_Vels(N);
         std::transform(control_list.begin(), control_list.end(),
                        ret_Vels.begin(),
                        [t, acc, &tot](auto & accMsg){
                            auto & [accTime, signal] = accMsg;
                            if(t <= accTime)             // 加速段
                            {
                                return signal * acc * t;
                            }
                            else if(t <= tot - accTime)  // 匀速段
                            {
                                return signal * acc * accTime;
                            }
                            else if(t <= tot - 1e-5)            // 减速段
                            {
                                return signal * acc * (tot - t);
                            }
                            else                        // 结束段
                            {
                                return 0.0;
                            }
                        }
         );
         return ret_Vels;
    };

    return VelocityFunction;
}
