#include "geometry/compute_penetration.h"
#include "geometry/convex_wrapper/convex_wrapper.h"
#include "GComponent/Geometry/gcollision_detection.h"

namespace GComponent{

#define CONTACT_PARAMS GJKOutput& output,							 \
					   AbstractShape* shape_a, CRefTransform pose_a, \
					   AbstractShape* shape_b, CRefTransform pose_b

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


static bool PenetrationSphereSphere		(CONTACT_PARAMS) {
	/*return IntersectSphereSphere(dynamic_cast<SphereShape*>(shape_a)->m_radius, 
								 pose_a.block(0, 3, 3, 1),
								 dynamic_cast<SphereShape*>(shape_b)->m_radius, 
								 pose_b.block(0, 3, 3, 1));*/
	return false;
}

static bool PenetrationSphereCapsule	(CONTACT_PARAMS) {
	SphereShape*  sphere  = dynamic_cast<SphereShape*>(shape_a);
	CapsuleShape* capsule = dynamic_cast<CapsuleShape*>(shape_b);
	/*return IntersectSphereCapsule(sphere->m_radius,  
								  pose_a.block(0, 3, 3, 1),
								  CAPSULE_PARAMS(capsule),
								  GetPoseRot_PARAMS(pose_b));*/
	return false;
}

static bool PenetrationSphereBox		(CONTACT_PARAMS) {
	SphereShape* sphere = dynamic_cast<SphereShape*>(shape_a);
	BoxShape*    box    = dynamic_cast<BoxShape*>(shape_b);
	/*return IntersectOBBSphere(BOX_PARAMS(box),
							  pose_a.block(0, 3, 3, 1), 
							  static_cast<SO3f>(pose_a.block(0, 0, 3, 3)),
							  sphere->m_radius, 
							  pose_b.block(0, 3, 3, 1));*/
	return false;
}

static bool PenetrationSpherePlane		(CONTACT_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool PenetrationSphereConvexHull	(CONTACT_PARAMS) {
	SphereShape* sphere = dynamic_cast<SphereShape*>(shape_a);
	ConvexShape* convex = dynamic_cast<ConvexShape*>(shape_b);

	ConvexSphere   capsule_hull = ConvexSphere(*sphere,   GetPoseRot_PARAMS(pose_a));
	ConvexHullMesh convex_hull  = ConvexHullMesh(*convex, GetPoseRot_PARAMS(pose_b));
		
	return PenetrationGjkEpa(output, capsule_hull, convex_hull, GetPose(pose_a) - GetPose(pose_b));
}

static bool PenetrationCapsuleCapsule	(CONTACT_PARAMS) {
	CapsuleShape* capsule_a = dynamic_cast<CapsuleShape*>(shape_a);
	CapsuleShape* capsule_b = dynamic_cast<CapsuleShape*>(shape_b);
	/*return IntersectCapsuleCapsule(CAPSULE_PARAMS(capsule_a),
								   GetPoseRot_PARAMS(pose_a),
								   CAPSULE_PARAMS(capsule_b),
								   GetPoseRot_PARAMS(pose_b));*/
	return false;
}

static bool PenetrationCapsuleBox		(CONTACT_PARAMS) {
	CapsuleShape* capsule = dynamic_cast<CapsuleShape*>(shape_a);
	BoxShape*	  box	  = dynamic_cast<BoxShape*>(shape_b);
	/*return IntersectOBBCapsule(BOX_PARAMS(box),
							   GetPoseRot_PARAMS(pose_b),
							   CAPSULE_PARAMS(capsule),
							   GetPoseRot_PARAMS(pose_a));*/
	return false;
}

static bool PenetrationCapsulePlane		(CONTACT_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool PenetrationCapsuleConvexHull(CONTACT_PARAMS) {
	CapsuleShape* capsule = dynamic_cast<CapsuleShape*>(shape_a);
	ConvexShape*  convex = dynamic_cast<ConvexShape*>(shape_b);
		
	ConvexCapsule  capsule_hull = ConvexCapsule(*capsule, GetPoseRot_PARAMS(pose_a));
	ConvexHullMesh convex_hull  = ConvexHullMesh(*convex, GetPoseRot_PARAMS(pose_b));
	
	return PenetrationGjkEpa(output, capsule_hull, convex_hull, GetPose(pose_a) - GetPose(pose_b));
}

static bool PenetrationBoxBox			(CONTACT_PARAMS) {
	BoxShape* box_a = dynamic_cast<BoxShape*>(shape_a);
	BoxShape* box_b = dynamic_cast<BoxShape*>(shape_b);

	/*return IntersectOBBOBB(BOX_PARAMS(box_a),
						   GetPoseRot_PARAMS(pose_a),
						   BOX_PARAMS(box_b),
						   GetPoseRot_PARAMS(pose_b));*/
	return false;
}

static bool PenetrationBoxPlane			(CONTACT_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool PenetrationBoxConvexHull	(CONTACT_PARAMS) {
	BoxShape*    box    = dynamic_cast<BoxShape*>(shape_a);
	ConvexShape* convex = dynamic_cast<ConvexShape*>(shape_b);

	ConvexBox		box_hull    = ConvexBox(*box, GetPoseRot_PARAMS(pose_a));
	ConvexHullMesh	convex_hull = ConvexHullMesh(*convex, GetPoseRot_PARAMS(pose_b));

	return PenetrationGjkEpa(output, box_hull, convex_hull, GetPose(pose_a) - GetPose(pose_b));
}

static bool PenetrationPlanePlane		(CONTACT_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool PenetrationPlaneConvexHull	(CONTACT_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool PenetrationConvexHullConvexHull(CONTACT_PARAMS) {
	ConvexShape* convex_a = dynamic_cast<ConvexShape*>(shape_a);
	ConvexShape* convex_b = dynamic_cast<ConvexShape*>(shape_b);

	ConvexHullMesh hull_a = ConvexHullMesh(*convex_a, GetPoseRot_PARAMS(pose_a));
	ConvexHullMesh hull_b = ConvexHullMesh(*convex_b, GetPoseRot_PARAMS(pose_b));

	return PenetrationGjkEpa(output, hull_a, hull_b, GetPose(pose_a) - GetPose(pose_b));
}

#undef CONTACT_PARAMS
#undef CAPSULE_PARAMS
#undef BOX_PARAMS
#undef GetPose
#undef GetRot
#undef GetPoseRot_PARAMS

/*____________________________COLLISION MAPS____________________________________*/
static std::map<ShapeEnum, MTDFunc> sphere_mtd_funcs = {
 {Sphere,		PenetrationSphereSphere},
 {Capsule,		PenetrationSphereCapsule},
 {Box,			PenetrationSphereBox},
 {Plane,		PenetrationSpherePlane},
 {ConvexHull,	PenetrationSphereConvexHull}
};

static std::map<ShapeEnum, MTDFunc> capsule_mtd_funcs = {
 {Sphere,		PenetrationSphereCapsule},
 {Capsule,		PenetrationCapsuleCapsule},
 {Box,			PenetrationCapsuleBox},
 {Plane,		PenetrationCapsulePlane},
 {ConvexHull,	PenetrationCapsuleConvexHull}
};

static std::map<ShapeEnum, MTDFunc> box_mtd_funcs	 = {
 {Sphere,		PenetrationSphereBox},
 {Capsule,		PenetrationCapsuleBox},
 {Box,			PenetrationBoxBox},
 {Plane,		PenetrationBoxPlane},
 {ConvexHull,	PenetrationBoxConvexHull}
};

static std::map<ShapeEnum, MTDFunc> plane_mtd_funcs = {
{Sphere,		PenetrationSpherePlane},
{Capsule,		PenetrationCapsulePlane},
{Box,			PenetrationBoxPlane},
{Plane,			PenetrationPlanePlane},
{ConvexHull,	PenetrationPlaneConvexHull}
};

static std::map<ShapeEnum, MTDFunc> convex_mtd_funcs = {
{Sphere,		PenetrationSphereConvexHull},
{Capsule,		PenetrationCapsuleConvexHull},
{Box,			PenetrationBoxConvexHull},
{Plane,			PenetrationPlaneConvexHull},
{ConvexHull,	PenetrationConvexHullConvexHull}
};

std::map<ShapeEnum, std::map<ShapeEnum, MTDFunc>>
CollisionPenetration::mtd_funcs = {
{Sphere,		sphere_mtd_funcs},
{Capsule,		capsule_mtd_funcs},
{Box,			box_mtd_funcs},
{Plane,			plane_mtd_funcs},
{ConvexHull,	convex_mtd_funcs}
};

bool GComponent::CollisionPenetration::Penetration(GJKOutput& mtd, CRefShapePtrs shapes_a, CRefTransform pose_a, CRefShapePtrs shapes_b, CRefTransform pose_b)
{
	for (auto shape_a : shapes_a) {
		ShapeEnum type_a = shape_a->GetShapeType();
		for (auto shape_b : shapes_b) {
			auto& mtd_func = mtd_funcs[type_a][shape_b->GetShapeType()];
			if (type_a <= shape_b->GetShapeType()) {
				if (mtd_func(mtd, shape_a, pose_a, shape_b, pose_b)) {
					return true;
				}
			}
			else {
				if (mtd_func(mtd, shape_b, pose_b, shape_a, pose_a)) {
					std::swap(mtd.closest_a, mtd.closest_b);
					mtd.normal	   = -mtd.normal;
					mtd.search_dir = -mtd.search_dir;
					return true;
				}
			}
		}
	}
	return false;
}

}