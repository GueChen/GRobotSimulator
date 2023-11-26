#include "motion/gmotionbase.h"

#include "component/transform_component.h"
#include "component/kinematic_component.h"
#include "component/joint_group_component.h"

#include <iostream>

namespace GComponent
{
/*_____________________________________________MotionBase Class____________________________________________*/
MotionBase& GComponent::MotionBase::SetMaxLinVel(float vel)
{
    max_vel_ = vel;
    return *this;
}

MotionBase& MotionBase::SetMaxLinAcc(float acc)
{
    max_acc_ = acc;
    return *this;
}

MotionBase& MotionBase::SetMaxAngVel(float vel)
{
    max_ang_vel_ = vel;
    return *this;    
}

MotionBase& MotionBase::SetMaxAngAcc(float acc)
{
    max_ang_acc_ = acc;
    return *this;
}

/*_____________________________________________CMotionBase Class____________________________________________*/
CTrajectory CMotionBase::operator()(Model* robot)
{
    KinematicComponent&  kinematic_sdk  = *robot->GetComponent<KinematicComponent>();        
    SE3f                 mat_ini;         kinematic_sdk.ForwardKinematic(mat_ini);
    TransformCom&           trans = *robot->GetTransform();
    mat_ini = trans.GetModelMatrixWithoutScale() * mat_ini;

    PathFunc path = PathFuncImpl(mat_ini, goal_);               // get path planning data
    time_total_   = ExecutionTimeImpl(mat_ini, goal_);          // caculate the total execution time
    
    time_total_ = time_total_  + 2;
    //std::cout << "time total : " << time_total_ << std::endl;
    
    return CTrajectory(*robot, time_total_, path);
}

CTrajectory CMotionBase::operator()(Model* robot, SE3f start)
{
    PathFunc path = PathFuncImpl(start, goal_);               // get path planning data
    time_total_   = ExecutionTimeImpl(start, goal_);          // caculate the total execution time

    time_total_ = time_total_  + 2;
    //std::cout << "time total : " << time_total_ << std::endl;
    return CTrajectory(*robot, time_total_, path);
}

/*_____________________________________________DualMotionBase Class_________________________________________*/
DualMotionBase& DualMotionBase::SetMaxLeftVel(float vel)
{
    left_max_vel_ = vel;
    return *this;
}

DualMotionBase& DualMotionBase::SetMaxLeftAcc(float acc)
{
    left_max_acc_ = acc;
    return *this;
}

DualMotionBase& DualMotionBase::SetMaxRightVel(float vel)
{
    right_max_vel_ = vel;
    return *this;
}

DualMotionBase& DualMotionBase::SetMaxRightAcc(float acc)
{
    right_max_acc_ = acc;
    return *this;
}


DualTrajectory DualSyncMotionBase::operator()(Model* left, Model* right)
{
    KinematicComponent& l_kine = *left->GetComponent<KinematicComponent>(),
                      & r_kine = *right->GetComponent<KinematicComponent>();
    TransformCom         & l_trans = *left->GetTransform(),
                      & r_trans = *right->GetTransform();
    SE3f l_ini, r_ini;
    l_kine.ForwardKinematic(l_ini); l_ini = l_trans.GetModelMatrixWithoutScale() * l_ini;
    r_kine.ForwardKinematic(r_ini); r_ini = r_trans.GetModelMatrixWithoutScale() * r_ini;
    SE3f l_goal = center_goal_ * left_trans_, r_goal = center_goal_ * right_trans_;
    
    auto&& [l_path, r_path] = PathFuncImpl(l_ini, l_goal, r_ini, r_goal);
    time_total_             = ExecutionTimeImpl(l_ini, l_goal, r_ini, r_goal);
    time_total_ += 2;
    return {CTrajectory(*left, time_total_, l_path), CTrajectory(*right, time_total_, r_path)};
}

/*_________________________________________DualSyncMotionBase Class_________________________________________*/
DualSyncMotionBase& DualSyncMotionBase::SetLeftTransform(SE3f left_mat)
{
    left_trans_ = left_mat;
    return *this;
}

DualSyncMotionBase& DualSyncMotionBase::SetRightTransform(SE3f right_mat)
{
    right_trans_ = right_mat;
    return *this;
}



} // !namespace GComponent