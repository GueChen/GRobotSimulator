/**
 *  @file  	skinsensor_optimizer.h
 *  @brief 	this class is a midleware to bridge the sensor and trjectory.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	July 13th, 2022
 **/
#ifndef _SKINSENSOR_OPTIMIZER_H
#define _SKINSENSOR_OPTIMIZER_H

#include "motion/optimization/trajectory_optimization.h"

namespace GComponent {

class SkinSensorOptimizer : public TgtOptimizer
{
public:
	SkinSensorOptimizer() = default;
	~SkinSensorOptimizer() = default;

	std::vector<float> Optimize(Model&, const Twistf& glb_t, const std::vector<float>& thetas) override;

	bool ConditionCheck(Model&) override;
};

class SkinSensorSimpleKeeperOptimizer : public TgtOptimizer
{
public:
	SkinSensorSimpleKeeperOptimizer()  = default;
	~SkinSensorSimpleKeeperOptimizer() = default;

	std::vector<float> Optimize(Model&, const Twistf& glb_t, const std::vector<float>& thetas) override;

	bool ConditionCheck(Model&) override;
};

class SkinSensorLineOptimizer : public TgtOptimizer 
{
public:
	SkinSensorLineOptimizer(Vec3 dir);
	~SkinSensorLineOptimizer() = default;
	
	virtual std::vector<float> Optimize(Model&, const Twistf& glb_t, const std::vector<float>& thetas) override;

	virtual bool ConditionCheck(Model&) override;

protected:
	Vec3 fwd_dir_		     = Vec3::Zero();
	bool is_blocking_	     = false;
	int  blocking_count_     = 0;
	int  solve_failed_count_ = 0;
};
} // !namespace GComponent

#endif // !_SKINSENSOR_OPTIMIZER_H
