/**
 *  @file  	trajectory_optimization.h
 *  @brief 	This File contains optimizer for trajectory optimizing.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 25th, 2022
 **/
#ifndef __TRAJECTORY_OPTIMIZATION_H
#define __TRAJECTORY_OPTIMIZATION_H

#include "physics/physics_datastructure.hpp"

#include "component/rigidbody_component.h"

#include <vector>
#include <memory>
#include <map>

namespace GComponent {

class Model;
class PhysicsScene;

class TgtOptimizer {
public:
	TgtOptimizer()			= default;
	~TgtOptimizer()			= default;
	virtual std::vector<float>
			Optimize(Model&, const Twistf& glb_t, const std::vector<float>& thetas) = 0;
	virtual bool ConditionCheck(Model&) = 0;
};

class PhysxCheckerOptimizer : public TgtOptimizer {
public:
	PhysxCheckerOptimizer()  = default;
	~PhysxCheckerOptimizer() = default;
	std::vector<float> 
			Optimize(Model&, const Twistf& glb_t, const std::vector<float>& thetas) override;
	bool	ConditionCheck(Model&) override;
	
private:
	bool	ConditionCheck(Model&, const std::shared_ptr<PhysicsScene>&, int);
	void	DisplayHitterInformations(GComponent::Model& obj);
private:
	std::map<int, std::vector<OverlapHitInfo>> hit_infos_;
	std::map<int, RigidbodyComponent&>		   hit_actors_;
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
