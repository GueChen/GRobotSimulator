/**
 *  @file  	gmotionbase.h
 *  @brief 	planning dialog responsible for interact with worker.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Dec xxth, 2021
 *          Aug 15th, 2022 reconstruct to fit the multiple configration robots
 *                         not coupling with KUKA_IIWA_ROBOT Class
 **/
#ifndef GMOTIONBASE_H
#define GMOTIONBASE_H

#include "model/model.h"
#include "motion/trajectory.h"

#include <GComponent/GTransform.hpp>
#include <GComponent/GGeometry.hpp>
#include <GComponent/GNumerical.hpp>

#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#include <execution>
#include <algorithm>
#include <functional>

namespace GComponent {

using std::function;

using std::string;
using std::tuple;
using std::map;
using std::vector;
using std::unordered_map;

enum class InterpolationEnum{
    Simple      = 0,
    Quadratic,
    Cubic,
    Quintic,
    Trapezoid
};

class MotionBase
{
public:
    MotionBase()                              = default;
    virtual ~MotionBase()                     = default;    
    
    MotionBase& SetMaxLinVel(float vel);
    MotionBase& SetMaxLinAcc(float acc);
    MotionBase& SetMaxAngVel(float vel);
    MotionBase& SetMaxAngAcc(float acc);

    inline float GetTotalTime() const { return time_total_; }
protected:
    float max_vel_     = -1.0f;
    float max_acc_     = -1.0f;
    float max_ang_vel_ = -1.0f;
    float max_ang_acc_ = -1.0f;
    float time_total_  = 0.0f;
};


class JMotionBase : public MotionBase {
public: 
    JMotionBase()                            = default;
    virtual ~JMotionBase()                   = default;
    virtual JTrajFunc operator()(Model* robot)  = 0;
};

class CMotionBase : public MotionBase {
public:
    explicit CMotionBase(const SE3f& goal):goal_(goal) {}
    virtual ~CMotionBase()                   = default;

//  Base class interface to get trajectory
    Trajectory operator()(Model* robot);
    Trajectory operator()(Model* robot, SE3f start);

protected:
//  Derived class overrivde implementation methods
    virtual PathFunc PathFuncImpl     (const SE3f& mat_ini, const SE3f& mat_end)  = 0;
    virtual float    ExecutionTimeImpl(const SE3f& mat_ini, const SE3f& mat_end)  = 0;

protected:
    SE3f goal_;
};


class DualMotionBase 
{
public:
    DualMotionBase()                          = default;
    virtual ~DualMotionBase()                 = default;    

    DualMotionBase& SetMaxLeftVel  (float vel);
    DualMotionBase& SetMaxLeftAcc  (float acc);
    DualMotionBase& SetMaxRightVel (float vel);
    DualMotionBase& SetMaxRightAcc (float acc);
    
    inline float GetTotalTime() const { return time_total_; }

protected:
    float time_total_    = 0.0f;
    float left_max_vel_  = -1.0f, left_max_acc_  = -1.0f;
    float right_max_vel_ = -1.0f, right_max_acc_ = -1.0f;
};


class DualSyncMotionBase : public DualMotionBase {
public:
    explicit DualSyncMotionBase(const SE3f& mat) : center_goal_(mat) {}
    virtual ~DualSyncMotionBase() = default;

    DualTrajectory operator()(Model* left, Model* right);

    DualSyncMotionBase& SetLeftTransform(SE3f left_mat);
    DualSyncMotionBase& SetRightTransform(SE3f right_mat);

protected:
    virtual 
    std::pair<PathFunc, PathFunc> PathFuncImpl(const SE3f& mat_ini_l, const SE3f& mat_end_l,
                                               const SE3f& mat_ini_r, const SE3f& mat_end_r) = 0;
    virtual
    float                         ExecutionTimeImpl
                                              (const SE3f& mat_ini_l, const SE3f& mat_end_l,
                                               const SE3f& mat_ini_r, const SE3f& mat_end_r) = 0;

protected:
    SE3f center_goal_;  
    SE3f left_trans_  = SE3f::Identity(), 
         right_trans_ = SE3f::Identity();
};

}
#endif // GMOTIONBASE_H
