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

class SkinSensorSimpleOptimizer : public TgtOptimizer
{
public:
	SkinSensorSimpleOptimizer()  = default;
	~SkinSensorSimpleOptimizer() = default;

	std::vector<float> Optimize(Model&, const Twistf& glb_t, const std::vector<float>& thetas) override;

	bool ConditionCheck(Model&) override;
};
} // !namespace GComponent

#endif // !_SKINSENSOR_OPTIMIZER_H
