/**
 *  @file  	trajectory.h
 *  @brief 	the trajectory class responsible for a model traj msg.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 24th, 2022
 **/
#ifndef __TRAJECTORY_H
#define __TRAJECTORY_H

#include "motion/optimization/trajectory_optimization.h"

#include <GComponent/GTransform.hpp>

#include <functional>
#include <optional>
#include <vector>
#include <memory>

namespace GComponent {

struct TrajectoryImpl;
class  Trajectory;
class  Model;

using PathFunc		 = std::function<Twistf(float)>;
using JointPair      = std::pair<float, float>;            
using JointPairs     = std::pair<std::vector<float>, std::vector<float>>;
using JTrajFunc      = std::function<JointPairs(float)>;
using DualJointPair  = std::pair<JointPairs, JointPairs>;
using DualTrajectory = std::pair<Trajectory, Trajectory>;

class Trajectory{
public:
	Trajectory(Model & obj, float time_total, PathFunc path_func);
	~Trajectory();

	JointPairs operator()(float t_reg);

	void			SetTimeTotal(float time_total);
	float			GetTimeTotal() const;

	inline SE3f		GetInitialPoint() const  { return ExpMapping(path_func_(0)); }
	inline SE3f     GetGoalPoint()	  const	 { return ExpMapping(path_func_(1)); }

	std::vector<std::vector<float>>
					GetSampleFromPathFunc(float interval);
	
	inline void		SetTargetOptimizer(TgtOptimizer* optimizer)		      { target_opt_ = std::shared_ptr<TgtOptimizer>(optimizer); }
	inline void		SetSelfMotionOptimizer(SelfmotionOptimizer* optimizer) { self_opt_ = std::shared_ptr<SelfmotionOptimizer>(optimizer); }
	Model&			GetModel();
	PathFunc		GetPathFunction();

protected:
	std::shared_ptr<TrajectoryImpl>		impl_		= nullptr;
	PathFunc							path_func_;

	std::shared_ptr<TgtOptimizer>		target_opt_ = nullptr;
	std::shared_ptr<SelfmotionOptimizer>self_opt_	= nullptr;

	//TODO: consider a better place
	Twistf								modify_vec_ = Twistf::Zero();

};	// !class Trajectory




} // !namespace GComponent

#endif // !__TRAJECTORY_H
