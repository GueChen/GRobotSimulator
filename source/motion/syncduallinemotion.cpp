#include "syncduallinemotion.h"
#include "model/robot/kuka_iiwa_model.h"
#include "function/conversion.hpp"

using namespace GComponent;

/// Constructor
SyncDualLineMotion::SyncDualLineMotion(const SE3d & T_goal): T_goal_(T_goal){}

/// Get Functor
DualJointMsgPkg
SyncDualLineMotion::GetCurvesFunction(KUKA_IIWA_MODEL* robot_left, KUKA_IIWA_MODEL* robot_right,
                                       const SE3d & T_bias_left,     const SE3d & T_bias_right,
                                       const double MaxVelLimitLeft, const double MaxVelLimitRight,
                                       const double MaxAccLimitLeft, const double MaxAccLimitRight)
{
    SE3d T_ini_left   = robot_left->ForwardKinematic(),
         T_ini_right  = robot_right->ForwardKinematic();
    IIWAThetas last_joint_left = robot_left->GetThetas(),
               last_joint_right= robot_right->GetThetas();

    auto &&[T_goal_left, T_goal_right] = GetTGoalTransforms(robot_left, robot_right, T_bias_left, T_bias_right);

    auto LineFuncLeft = GetScrewLineFunction(T_ini_left,  T_goal_left),
         LineFuncRight= GetScrewLineFunction(T_ini_right, T_goal_right);

    double scaler = std::max((T_ini_left.block(0, 3, 3, 1) - T_goal_left.block(0, 3, 3, 1)).norm(),
                             (T_ini_right.block(0, 3, 3, 1)- T_goal_right.block(0, 3, 3, 1)).norm());
    double tot = scaler / MaxVelLimitLeft;

    // FIXME: 未考虑末端轨迹的连续性，仅确保轨迹的正确性
    auto PositionFunction = [robot_left, robot_right, LineFuncLeft, LineFuncRight, tot, last_joint_left, last_joint_right](double t)mutable{
        double scalered_t = Clamp(t / tot, 0.0, 1.0);
        twistd twist_cur_left = LineFuncLeft(scalered_t),
               twist_cur_right= LineFuncRight(scalered_t);
        IIWAThetav vec_last_jp_left= Eigen::Map<IIWAThetav>(last_joint_left.data(), 7),
                   vec_last_jp_right= Eigen::Map<IIWAThetav>(last_joint_right.data(), 7);
        IIWAThetas ret_jp_left = robot_left->BackKinematic(twist_cur_left, vec_last_jp_left),
                   ret_jp_right= robot_right->BackKinematic(twist_cur_right, vec_last_jp_right);

        last_joint_left = ret_jp_left;
        last_joint_right= ret_jp_right;

        for(auto & jp: ret_jp_left)  jp = RadiusToDegree(ToStandarAngle(jp));
        for(auto & jp: ret_jp_right) jp = RadiusToDegree(ToStandarAngle(jp));

        return std::make_pair(JointPos(ret_jp_left.cbegin(), ret_jp_left.cend()),
                              JointPos(ret_jp_right.cbegin(), ret_jp_right.cend()));
    };

    return {tot, PositionFunction};
}

tuple<SE3d, SE3d>
SyncDualLineMotion::GetTGoalTransforms(KUKA_IIWA_MODEL* p_robot_left, KUKA_IIWA_MODEL* p_robot_right,
                                       const SE3d & T_bias_left, const SE3d & T_bias_right){
    mat4 base_left    = p_robot_left->getModelMatrix(),
         base_right   = p_robot_right->getModelMatrix();

    SE3d T_goal_left  = InverseSE3(Conversion::toMat4d(base_left)) * T_goal_ * T_bias_left,
         T_goal_right =InverseSE3(Conversion::toMat4d(base_right)) * T_goal_ * T_bias_right;

    return {T_goal_left, T_goal_right};
}
