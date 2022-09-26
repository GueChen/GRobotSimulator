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
class  CTrajectory;
class  Model;

using PathFunc		 = std::function<Twistf(float)>;
using JointPair      = std::pair<float, float>;            
using JointPairs     = std::pair<std::vector<float>, std::vector<float>>;
using JTrajFunc      = std::function<JointPairs(float)>;
using DualJointPair  = std::pair<JointPairs, JointPairs>;
using DualTrajectory = std::pair<CTrajectory, CTrajectory>;

enum TrajectoryType {
	eNone = 0, eJSpace = 1, eCSpace
};

class Trajectory {
public:
	// type alias
	using _Point3  = Eigen::Vector3f;
	using _Point3s = std::vector<_Point3>;
	template<class T>
	using _Ptr	   = std::shared_ptr<T>;
public: 
	Trajectory(Model& obj, float time_total);
	virtual			   ~Trajectory();
	
	inline TrajectoryType 
					   GetType() const { return type_; }

	void			   SetTimeTotal(float time_total);
	float			   GetTimeTotal()    const;
					   
	Model&			   GetModel();
	std::string		   GetIdentifier()	 const;
	
	[[nodiscard]]
	virtual Trajectory*GetPtrCopy()		 const = 0;

	virtual JointPairs operator()(float t_reg) = 0;

	virtual SE3f	   GetInitialPoint() const = 0;
	virtual SE3f       GetGoalPoint()	 const = 0;

	virtual _Point3s   GetSampleFromPathFunc(float interval) = 0;
		
protected:
	_Ptr<TrajectoryImpl>		impl_ = nullptr;
	TrajectoryType				type_ = eNone;
};

class JTrajectory : public Trajectory{
using _CurveFnc = std::function<JointPairs(float)>;
public:
	JTrajectory(Model& obj, float time_total, _CurveFnc curve_func);
	~JTrajectory();
	
	virtual JointPairs  operator()(float t_reg) override;
	virtual SE3f	    GetInitialPoint() const override;
	virtual SE3f	    GetGoalPoint()    const override;
	virtual _Point3s    GetSampleFromPathFunc(float interval) override;
	[[nodiscard]]
	virtual Trajectory* GetPtrCopy()     const override;
private:
	_CurveFnc fnc_;
};

class CTrajectory : public Trajectory{
	using _Base = Trajectory;

public:
	CTrajectory(Model & obj, float time_total, PathFunc path_func);
	~CTrajectory();

	virtual JointPairs operator()(float t_reg) override;
	
	virtual SE3f	GetInitialPoint() const  override;
	virtual SE3f    GetGoalPoint()	  const	 override;

	virtual _Base::_Point3s
					GetSampleFromPathFunc(float interval) override;
		
	[[nodiscard]]
	virtual Trajectory*
					GetPtrCopy()	  const  override;

	// path deserve function
	inline Twistf	GetModifyVector() const { return modify_vec_; }
	PathFunc		GetPathFunction();
	// opt cspace function
	inline void		SetTargetOptimizer(TgtOptimizer* optimizer)		{ target_opt_ = std::shared_ptr<TgtOptimizer>(optimizer); }
	inline void		SetSelfMotionOptimizer(SlfOptimizer* optimizer) { self_opt_   = std::shared_ptr<SlfOptimizer>(optimizer); }

protected:	
	PathFunc							func_;

	_Base::_Ptr<TgtOptimizer>			target_opt_ = nullptr;
	_Base::_Ptr<SlfOptimizer>			self_opt_	= nullptr;
	
	Twistf								modify_vec_ = Twistf::Zero();
};	// !class Trajectory

} // !namespace GComponent

#endif // !__TRAJECTORY_H
