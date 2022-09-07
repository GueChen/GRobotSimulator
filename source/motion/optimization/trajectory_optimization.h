/**
 *  @file  	trajectory_optimization.h
 *  @brief 	This File contains optimizer for trajectory optimizing.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 25th, 2022
 **/
#ifndef __TRAJECTORY_OPTIMIZATION_H
#define __TRAJECTORY_OPTIMIZATION_H

#include "physics/physics_datastructure.hpp"

#include <GComponent/GTransform.hpp>

#include <vector>
#include <memory>
#include <map>

namespace GComponent {

class Model;
class PhysicsScene;

class TargetOptimizer {
public:
	TargetOptimizer()  = default;
	~TargetOptimizer() = default;
	Twistf Optimize(Model&, const Twistf& glb_t, const std::vector<float>& thetas);	
	bool ConditionCheck(Model&);

private:
	bool ConditionCheck(Model&, const std::shared_ptr<PhysicsScene>&, int);

private:
	std::map<int, std::vector<OverlapHitInfo>> hit_infos;
};

class SelfmotionOptimizer {
public:
	SelfmotionOptimizer()  = default;
	~SelfmotionOptimizer() = default;
	Eigen::Vector<float, Eigen::Dynamic>
		 IncVector(Model&, const std::vector<float>& thetas);
	bool ConditionCheck(Model&);

};

} // !namespace GComponent

#endif // !__TRAJECTORY_OPTIMIZATION_H
