#include "linmotion.h"
#include "model/robot/kuka_iiwa_model.h"

#include <memory>
using std::forward;
using std::make_pair;
using namespace GComponent;

LinMotion::LinMotion(const SE3d & T):
    T_goal(T), obst_Dict()
{};


JointCruveMsgPkg LinMotion::GetCurvesFunction(KUKA_IIWA_MODEL * robot, const double Max_Vel_Limit, const double Max_Acc_Limit)
{

    SE3d T_ini = robot->ForwardKinematic();
    IIWAThetas lastJoint = robot->GetThetas();
    const size_t JointNum = robot->GetThetas().size();

    auto LineFunc = GetScrewLineFunction(T_ini, T_goal);
    double scaler = (T_ini.block(0, 3, 3, 1) - T_goal.block(0, 3, 3, 1)).norm();
    double tot = scaler / Max_Vel_Limit;

    /* 没有任何连续性考虑的测试 */
    auto PositionFunction = [robot, Max_Vel_Limit, LineFunc, tot, lastJp = lastJoint](double t)mutable{
        double scalered_t  = Clamp(t / tot, 0.0, 1.0);
        Twistd twist_cur   = LineFunc(scalered_t);
        IIWAThetav lastJpv =Eigen::Map<IIWAThetav>(lastJp.data(), 7);
        IIWAThetas ret_Pos = robot->BackKinematic(twist_cur, lastJpv);
        lastJp = ret_Pos;
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
        Twistd v_thetav;
        for(int i = 0; i < 6; ++i)
        {
            v_thetav[i] = Max_Vel_Limit;
        }

        return vector<double>(7, Max_Vel_Limit);
    };
    return {tot, PositionFunction, VelocityFunction};
}

// TODO: 考虑去除重复的部分
JointCruveMsgPkg
LinMotion::GetAvoidObstCurvesFunction(KUKA_IIWA_MODEL* robot, double Padding, const double Max_Vel_Limit, const double Max_Acc_Limit)
{
    SE3d T_ini = robot->ForwardKinematic();
    const size_t JointNum = robot->GetThetas().size();

    auto SimpleLineFunc = GetScrewLineFunction(T_ini, T_goal);
    auto AvoidOptLineFunc = [obst_Dict = this->obst_Dict, SimpleLineFunc, Padding](double t)->Twistd{

        Twistd twist_ori = SimpleLineFunc(t);
        SE3d T           = ExpMapping(twist_ori);
        const Vec3d & pos_Ori = T.block(0, 3, 3, 1);
        Vec3d posNew = Vec3d::Zero();
        for(auto & [name, obst]: obst_Dict)
        {
            auto && [center, radius] = obst;
            if((center - pos_Ori).norm() < radius + Padding)
            {
                Vec3d direction  = (pos_Ori - center).normalized();
                double safeDist  = radius + Padding;
                posNew = center + safeDist * direction;
                break;
            }
        }
        if(posNew.norm() != 0)
        {
            T.block(0, 3, 3, 1) = posNew;
            return LogMapSE3Tose3(T);
        }
        return twist_ori;
    };

    double scaler = (T_ini.block(0, 3, 3, 1) - T_goal.block(0, 3, 3, 1)).norm();
    double tot = scaler / Max_Vel_Limit;
    vector<BallObstacle> obsts(obst_Dict.size());
    std::transform(obst_Dict.begin(), obst_Dict.end(), obsts.begin(), [](auto && dict)
    {
        auto & [name, obst] = dict;
        return obst;
    });
    /* 没有任何连续性考虑的测试 */
    auto PositionFunction = [robot, obsts = obsts, Max_Vel_Limit, LineFunc = AvoidOptLineFunc, tot](double t){
        double scalered_t  = Clamp(t / tot, 0.0, 1.0);
        // 该 LinFunction 已是末端单点避障的 line function
        Twistd twist_cur   = LineFunc(scalered_t);
        // 求逆解求出目标点关节值
        IIWAThetas ret_Pos = robot->BackKinematic(twist_cur);
        // 检测中间是否有避障需求
        int Iter = 0;
        while( robot->CheckCollisionSafe(obsts, ret_Pos)&& Iter++ < 50)
        {
            double beforeVal = robot->GetCollisionVal(obsts, ret_Pos);

            vec7d grad = robot->GetCollisionGrad(obsts, ret_Pos);

            IIWAThetas retPos = robot->SelfMotion(100.0 * grad, retPos);
            double afterVal = robot->GetCollisionVal(obsts, ret_Pos);
            if(std::abs(afterVal - beforeVal) > 1e-9)
            {
                std::cout << "before val:" << beforeVal << std::endl;
                std::cout << "after  val:" << afterVal  << std::endl;
            }
            else if(Iter == 1)
            {
                std::cout << " The grad =\n " << grad << std::endl;

            }
        }
        // FIXME:
        /*
         *如果替换为 WLN
         *  robot->GetWLNGrad ? 这样接口会不会太繁杂?
         *  且同样是 GPM 但避关节极限接口要写两个? 且二者没有联系?
          */

        // 赋值处理
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
        Twistd v_thetav;
        for(int i = 0; i < 6; ++i)
        {
            v_thetav[i] = Max_Vel_Limit;
        }

        return vector<double>(7, Max_Vel_Limit);
    };

    return {tot, PositionFunction, VelocityFunction};
}

JointCruveMsgPkg
LinMotion::GetAvoidLimitationCurvesFunction(KUKA_IIWA_MODEL* robot, const double Max_Vel_Limit, const double Max_Acc_Limit)
{
    SE3d T_ini = robot->ForwardKinematic();
    IIWAThetas last_joint = robot->GetThetas();
    const unsigned joint_num = last_joint.size();
    Matrix<double, joint_num, 1> last_grad = robot->GetJointsLimitationGrad();
    auto LineFunc = GetScrewLineFunction(T_ini, T_goal);

    double scaler = (T_ini.block(0, 3, 3, 1) - T_goal.block(0, 3, 3, 1)).norm();
    double tot = scaler / Max_Vel_Limit;
    /* 没有任何连续性考虑的测试 */
    auto PositionFunction = [robot, Max_Vel_Limit, joint_num = joint_num, LineFunc, tot, last_jp = last_joint, last_gd = last_grad](double t)mutable{
        double scalered_t  = Clamp(t / tot, 0.0, 1.0);
        Twistd twist_cur   = LineFunc(scalered_t);
        IIWAThetav lastJpv =Eigen::Map<IIWAThetav>(last_jp.data(), joint_num);
        Matrix<double, 7, 1> grad = robot->GetJointsLimitationGrad();
        Matrix<double, 7, 7> weighted_matrix = Matrix<double, 7, 7>::Identity();
        for(int i = 0; i < joint_num; ++i){
            if(!(std::abs(grad(i, 0)) - std::abs(last_gd(i, 0)) < 0)){
                weighted_matrix(i, i) += 5.0 * std::abs(grad(i, 0));
            }
        }
        last_gd = grad;
        IIWAThetas ret_Pos  = robot->WeightedBackKinematic(weighted_matrix, twist_cur, lastJpv);
        last_jp = ret_Pos;
        std::for_each(ret_Pos.begin(), ret_Pos.end(),[](auto & num)
        {
            num = ToStandarAngle(num);
            num = RadiusToDegree(num);
        });

        std::cout << std::endl;
        return vector<double>(ret_Pos.cbegin(), ret_Pos.cend());
    };
    auto VelocityFunction = [robot, Max_Vel_Limit, tot, joint_num, PositionFunction](double t){
        if(t <= tot || t >= tot) return vector<double>(7, 0.0);
        Twistd v_thetav;
        for(int i = 0; i < 6; ++i)
        {
            v_thetav[i] = Max_Vel_Limit;
        }

        return vector<double>(7, Max_Vel_Limit);
    };
    return {tot, PositionFunction, VelocityFunction};
}

void LinMotion::addObstacle(const string & name, BallObst && obst)
{
    deleteObstacle(name);
    obst_Dict.insert(make_pair(name, forward<BallObst>(obst)));
}

void LinMotion::updateObstacle(const string & name, Vec3d pos_new)
{
    if(obst_Dict.count(name))
    {
        obst_Dict[name].first = pos_new;
    }
}

void LinMotion::deleteObstacle(const string & name)
{
    if(obst_Dict.count(name))
    {
        obst_Dict.erase(name);
    }
}

