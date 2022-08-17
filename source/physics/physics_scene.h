/**
 *  @file  	physics_scene.h
 *  @brief 	This file is a wrapper for physics scene.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 8th, 2022
 **/

#ifndef __PHYSICS_SCENE
#define __PHYSICS_SCENE

#include "physics/abstract_shape.hpp"
#include "physics/rigidbody_actor.h"

#include <Eigen/Dense>
#include <memory>
#include <vector>

namespace GComponent {

using std::vector;

struct PhysicsImpl;

///TODO: consider putting hit info to a proper place
struct OverlapHitInfo {
	uint64_t	actor_id;
	Vec3		penetration_vec;
};

struct SweepHitInfo {

};

struct RaycastHitInfo {

};

class PhysicsScene {
public:
	PhysicsScene();
	~PhysicsScene();

	void tick(float time_step);

	RigidBodyActor* CreateRigidBodyActor(const Mat4& global_mat, const Mat4& local_mat, AbstractShape& shape_res, CollisionGroup group = CollisionGroup{});
	void			RemoveRigidBodyActor(RigidBodyActor* actor);

	void			UpdateRigidBodyActorTransfrom(RigidBodyActor* actor, const Mat4& mat);

	bool			Overlap(RigidBodyActor& actor, uint32_t max_hits, vector<OverlapHitInfo>& hits);
	bool			Sweep(RigidBodyActor& actor, const Vec3& dir, float dist, vector<SweepHitInfo>& hits);
	bool			Raycast(Vec3 ori, Vec3 dir, float max_dist, vector<RaycastHitInfo>& hits);
protected:
	shared_ptr<PhysicsImpl>	physics_impl_	= nullptr;	
};


} // !namespace GComponent

#endif // !__PHYSICS_SCENE
