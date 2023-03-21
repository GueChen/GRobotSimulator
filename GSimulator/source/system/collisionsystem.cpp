#include "system/collisionsystem.h"

#include "geometry/convex_wrapper/convex_wrapper.h"

#include "GComponent/Geometry/gcollision_detection.h"

#include <iostream>

namespace GComponent {

/*____________________________________STATIC METHODS___________________________________*/
#define OVERLAP_PARAMS AbstractShape* shape_a, CollisionSystem::CRefTransform pose_a, \
					   AbstractShape* shape_b, CollisionSystem::CRefTransform pose_b
#define CAPSULE_PARAMS(cap_ptr)	\
	(cap_ptr)->m_radius,		\
	(cap_ptr)->m_half_height

#define BOX_PARAMS(box_ptr)		\
	Vec3f((box_ptr)->m_half_x, (box_ptr)->m_half_y, (box_ptr)->m_half_z)

#define GetPose(Trans)			\
	(Trans).block(0, 3, 3, 1)

#define GetRot(Trans)			\
	static_cast<SO3f>((Trans).block(0, 0, 3, 3))

#define GetPoseRot_PARAMS(Trans)\
	(Trans).block(0, 3, 3, 1),  \
	static_cast<SO3f>((Trans).block(0, 0, 3, 3))




static bool OverlapSphereSphere		(OVERLAP_PARAMS) {
	return IntersectSphereSphere(dynamic_cast<SphereShape*>(shape_a)->m_radius, 
								 pose_a.block(0, 3, 3, 1),
								 dynamic_cast<SphereShape*>(shape_b)->m_radius, 
								 pose_b.block(0, 3, 3, 1));
}

static bool OverlapSphereCapsule	(OVERLAP_PARAMS) {
	SphereShape*  sphere  = dynamic_cast<SphereShape*>(shape_a);
	CapsuleShape* capsule = dynamic_cast<CapsuleShape*>(shape_b);
	return IntersectSphereCapsule(sphere->m_radius,  
								  pose_a.block(0, 3, 3, 1),
								  CAPSULE_PARAMS(capsule),
								  GetPoseRot_PARAMS(pose_b));
}

static bool OverlapSphereBox		(OVERLAP_PARAMS) {
	SphereShape* sphere = dynamic_cast<SphereShape*>(shape_a);
	BoxShape*    box    = dynamic_cast<BoxShape*>(shape_b);
	return IntersectOBBSphere(BOX_PARAMS(box),
							  pose_a.block(0, 3, 3, 1), 
							  static_cast<SO3f>(pose_a.block(0, 0, 3, 3)),
							  sphere->m_radius, 
							  pose_b.block(0, 3, 3, 1));
}

static bool OverlapSpherePlane		(OVERLAP_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool OverlapSphereConvexHull	(OVERLAP_PARAMS) {
	SphereShape* sphere = dynamic_cast<SphereShape*>(shape_a);
	ConvexShape* convex = dynamic_cast<ConvexShape*>(shape_b);
	return IntersectGJK(ConvexSphere  (*sphere, GetPoseRot_PARAMS(pose_a)), 
						ConvexHullMesh(*convex, GetPoseRot_PARAMS(pose_b)),
					    GetPose(pose_a) - GetPose(pose_b));
}

static bool OverlapCapsuleCapsule	(OVERLAP_PARAMS) {
	CapsuleShape* capsule_a = dynamic_cast<CapsuleShape*>(shape_a);
	CapsuleShape* capsule_b = dynamic_cast<CapsuleShape*>(shape_b);
	return IntersectCapsuleCapsule(CAPSULE_PARAMS(capsule_a),
								   GetPoseRot_PARAMS(pose_a),
								   CAPSULE_PARAMS(capsule_b),
								   GetPoseRot_PARAMS(pose_b));
}

static bool OverlapCapsuleBox		(OVERLAP_PARAMS) {
	CapsuleShape* capsule = dynamic_cast<CapsuleShape*>(shape_a);
	BoxShape*	  box	  = dynamic_cast<BoxShape*>(shape_b);
	return IntersectOBBCapsule(BOX_PARAMS(box),
							   GetPoseRot_PARAMS(pose_b),
							   CAPSULE_PARAMS(capsule),
							   GetPoseRot_PARAMS(pose_a));
}

static bool OverlapCapsulePlane		(OVERLAP_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool OverlapCapsuleConvexHull(OVERLAP_PARAMS) {
	return false;
}

static bool OverlapBoxBox			(OVERLAP_PARAMS) {
	BoxShape* box_a = dynamic_cast<BoxShape*>(shape_a);
	BoxShape* box_b = dynamic_cast<BoxShape*>(shape_b);
	return IntersectOBBOBB(BOX_PARAMS(box_a),
						   GetPoseRot_PARAMS(pose_a),
						   BOX_PARAMS(box_b),
						   GetPoseRot_PARAMS(pose_b));
}

static bool OverlapBoxPlane			(OVERLAP_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool OverlapBoxConvexHull	(OVERLAP_PARAMS) {
	return false;
}

static bool OverlapPlanePlane		(OVERLAP_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool OverlapPlaneConvexHull	(OVERLAP_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool OverlapConvexHullConvexHull(OVERLAP_PARAMS) {
	return false;
}

/*____________________________COLLISION MAPS____________________________________*/
static std::map<ShapeEnum, CollisionSystem::CollisionFunc> sphere_col_map = {
 {Sphere,		OverlapSphereSphere},
 {Capsule,		OverlapSphereCapsule},
 {Box,			OverlapSphereBox},
 {Plane,		OverlapSpherePlane},
 {ConvexHull,	OverlapSphereConvexHull}
};

static std::map<ShapeEnum, CollisionSystem::CollisionFunc> capsule_col_map = {
 {Sphere,		OverlapSphereCapsule},
 {Capsule,		OverlapCapsuleCapsule},
 {Box,			OverlapCapsuleBox},
 {Plane,		OverlapCapsulePlane},
 {ConvexHull,	OverlapCapsuleConvexHull}
};

static std::map<ShapeEnum, CollisionSystem::CollisionFunc> box_col_map	 = {
 {Sphere,		OverlapSphereBox},
 {Capsule,		OverlapCapsuleBox},
 {Box,			OverlapBoxBox},
 {Plane,		OverlapBoxPlane},
 {ConvexHull,	OverlapBoxConvexHull}
};

static std::map<ShapeEnum, CollisionSystem::CollisionFunc> plane_col_map = {
{Sphere,		OverlapSpherePlane},
{Capsule,		OverlapCapsulePlane},
{Box,			OverlapBoxPlane},
{Plane,			OverlapPlanePlane},
{ConvexHull,	OverlapPlaneConvexHull}
};

static std::map<ShapeEnum, CollisionSystem::CollisionFunc> convex_col_map = {
{Sphere,		OverlapSphereConvexHull},
{Capsule,		OverlapCapsuleConvexHull},
{Box,			OverlapBoxConvexHull},
{Plane,			OverlapPlaneConvexHull},
{ConvexHull,	OverlapConvexHullConvexHull}
};

std::map<ShapeEnum, std::map<ShapeEnum, CollisionSystem::CollisionFunc>>
CollisionSystem::collision_func_map = {
{Sphere,		sphere_col_map},
{Capsule,		capsule_col_map},
{Box,			box_col_map},
{Plane,			plane_col_map},
{ConvexHull,	convex_col_map}
};

/*_____________________COLLISION SYSTEMS CLASS________________________________*/
/*_____________________PUBLIC METHODS_________________________________________*/
CollisionSystem& CollisionSystem::getInstance()
{
	static CollisionSystem instance;
	return instance;
}

CollisionSystem::~CollisionSystem() = default;

void CollisionSystem::Initialize()
{}

void CollisionSystem::AddProcessShapes(CRefShapePtrs shapes, CRefTransform pose)
{
	need_process_.push_back(std::make_pair(shapes, pose));
}

void CollisionSystem::tick(float delta_time)
{
	(void)delta_time;
	//TODO: add broad phase collision checking to accelerate the whole process
	for (int i = 0; i < need_process_.size(); ++i) {
		auto& [col_a, pose_a] = need_process_[i];
	for (int j = i + 1; j < need_process_.size(); ++j) {
		auto& [col_b, pose_b] = need_process_[j];
		OverlapCheck(col_a, pose_a, col_b, pose_b);
	}
	}

	need_process_.clear();	
}

void CollisionSystem::OverlapCheck(CRefShapePtrs shapes_a, Transform pose_a,
								   CRefShapePtrs shapes_b, Transform pose_b)
{
	for (auto shape_a : shapes_a) {
		ShapeEnum type_a = shape_a->GetShapeType();
		for (auto shape_b : shapes_b) {
			auto& collision_func = collision_func_map[type_a][shape_b->GetShapeType()];
			bool need_swap = false;
			if (type_a > shape_b->GetShapeType()) {
				need_swap = true;
				std::swap(shape_a, shape_b);
				std::swap(pose_a,  pose_b);
			}

			if (collision_func(shape_a, pose_a, shape_b, pose_b)) {
				std::cout << "collision happened\n";
			}

			if (need_swap) {
				std::swap(pose_a, pose_b);
			}
		}
	}
}



}