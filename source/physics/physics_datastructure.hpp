/**
 *  @file  	physics_datastructure.h
 *  @brief 	some structure using to describe the hitting informations.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 25th, 2022
 **/
#ifndef __PHYSICS_DATASTRUCTURE_H
#define __PHYSICS_DATASTRUCTURE_H

#include <Eigen/Core>
#include <cstdint>

namespace GComponent {

struct OverlapHitInfo {
	uint64_t		 actor_id;
	Eigen::Vector3f	 penetration_vec;
};

struct SweepHitInfo {

};

struct RaycastHitInfo {

};

} // !namespace GComponent

#endif // !__PHYSICS_DATASTRUCTURE_H
