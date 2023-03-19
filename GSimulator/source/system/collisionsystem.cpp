#include "system/collisionsystem.h"

#include "GComponent/Geometry/gcollision_detection.h"

#include <iostream>

namespace GComponent {

/*____________________________________STATIC METHODS___________________________________*/
#define OVERLAP_PARAMS AbstractShape* shape_a, CollisionSystem::CRefTransform pose_a, \
					   AbstractShape* shape_b, CollisionSystem::CRefTransform pose_b

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
								  capsule->m_radius, 
								  capsule->m_half_height, 
								  pose_b.block(0, 3, 3, 1), 
								  static_cast<SO3f>(pose_b.block(0, 0, 3, 3)));
}

static bool OverlapSphereBox		(OVERLAP_PARAMS) {
	SphereShape* sphere = dynamic_cast<SphereShape*>(shape_a);
	BoxShape*    box    = dynamic_cast<BoxShape*>(shape_b);
	return IntersectOBBSphere(Vec3f(box->m_half_x, box->m_half_y, box->m_half_z), 
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
	return false
		//IntersectGJK(Convex(Sphere), Convex(Convex))
		;
}

static bool OverlapCapsuleCapsule	(OVERLAP_PARAMS) {

	return false;
}

static bool OverlapCapsuleBox		(OVERLAP_PARAMS) {
	return false;
}

static bool OverlapCapsulePlane		(OVERLAP_PARAMS) {
	return false;
}

static bool OverlapCapsuleConvexHull(OVERLAP_PARAMS) {
	return false;
}

static bool OverlapBoxBox			(OVERLAP_PARAMS) {
	return false;
}

static bool OverlapBoxPlane			(OVERLAP_PARAMS) {
	return false;
}

static bool OverlapBoxConvexHull	(OVERLAP_PARAMS) {
	return false;
}

static bool OverlapPlanePlane		(OVERLAP_PARAMS) {
	return false;
}

static bool OverlapPlaneConvexHull	(OVERLAP_PARAMS) {
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

void CollisionSystem::OverlapCheck(CRefShapePtrs shapes_a, CRefTransform pose_a,
								   CRefShapePtrs shapes_b, CRefTransform pose_b)
{
	for (auto& shape_a : shapes_a) {
		ShapeEnum type_a = shape_a->GetShapeType();
		for (auto& shape_b : shapes_b) {
			auto& collision_func = collision_func_map[type_a][shape_b->GetShapeType()];
			if (collision_func(shape_a, pose_a, shape_b, pose_b)) {
				std::cout << "collision happened\n";
			}
		}
	}
}



}