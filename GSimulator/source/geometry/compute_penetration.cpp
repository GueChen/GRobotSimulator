#include "geometry/compute_penetration.h"
#include "geometry/convex_wrapper/convex_wrapper.h"
#include "GComponent/Geometry/gcollision_detection.h"

namespace GComponent{

#define CONTACT_PARAMS PenetrationOutput& output,							 \
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


static bool MTDSphereSphere		(CONTACT_PARAMS) {
	SphereShape* sphere_a = dynamic_cast<SphereShape*>(shape_a);
	SphereShape* sphere_b = dynamic_cast<SphereShape*>(shape_b);
	return PenetrationSphereSphere(output,
								   sphere_a->m_radius, GetPose(pose_a),
								   sphere_b->m_radius, GetPose(pose_b));	
}

static bool MTDSphereCapsule	(CONTACT_PARAMS) {
	SphereShape*  sphere  = dynamic_cast<SphereShape*>(shape_a);
	CapsuleShape* capsule = dynamic_cast<CapsuleShape*>(shape_b);
	return PenetrationSphereCapsule(output, 
								    sphere->m_radius, GetPose(pose_a),
								    CAPSULE_PARAMS(capsule),
								    GetPoseRot_PARAMS(pose_b));
}

static bool MTDSphereBox		(CONTACT_PARAMS) {
	SphereShape* sphere = dynamic_cast<SphereShape*>(shape_a);
	BoxShape*    box    = dynamic_cast<BoxShape*>(shape_b);
	return PenetrationOBBSphere(output,
								BOX_PARAMS(box),  GetPoseRot_PARAMS(pose_b),								
								sphere->m_radius, GetPose(pose_a));	
}

static bool MTDSpherePlane		(CONTACT_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool MTDSphereConvexHull	(CONTACT_PARAMS) {
	SphereShape* sphere = dynamic_cast<SphereShape*>(shape_a);
	ConvexShape* convex = dynamic_cast<ConvexShape*>(shape_b);

	ConvexSphere   capsule_hull = ConvexSphere(*sphere,   GetPoseRot_PARAMS(pose_a));
	ConvexHullMesh convex_hull  = ConvexHullMesh(*convex, GetPoseRot_PARAMS(pose_b));
		
	return PenetrationGjkEpa(output, capsule_hull, convex_hull, GetPose(pose_a) - GetPose(pose_b));
}

static bool MTDCapsuleCapsule	(CONTACT_PARAMS) {
	CapsuleShape* capsule_a = dynamic_cast<CapsuleShape*>(shape_a);
	CapsuleShape* capsule_b = dynamic_cast<CapsuleShape*>(shape_b);
	return PenetrationCapsuleCapsule(output,
									 CAPSULE_PARAMS(capsule_a), GetPoseRot_PARAMS(pose_a),
								     CAPSULE_PARAMS(capsule_b), GetPoseRot_PARAMS(pose_b));
}

static bool MTDCapsuleBox		(CONTACT_PARAMS) {
	CapsuleShape* capsule = dynamic_cast<CapsuleShape*>(shape_a);
	BoxShape*	  box	  = dynamic_cast<BoxShape*>(shape_b);
	return PenetrationOBBCapsule(output,
								 BOX_PARAMS(box),		  GetPoseRot_PARAMS(pose_b),
							     CAPSULE_PARAMS(capsule), GetPoseRot_PARAMS(pose_a));	
}

static bool MTDCapsulePlane		(CONTACT_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool MTDCapsuleConvexHull(CONTACT_PARAMS) {
	CapsuleShape* capsule = dynamic_cast<CapsuleShape*>(shape_a);
	ConvexShape*  convex = dynamic_cast<ConvexShape*>(shape_b);
		
	ConvexCapsule  capsule_hull = ConvexCapsule(*capsule, GetPoseRot_PARAMS(pose_a));
	ConvexHullMesh convex_hull  = ConvexHullMesh(*convex, GetPoseRot_PARAMS(pose_b));
	
	return PenetrationGjkEpa(output, capsule_hull, convex_hull, GetPose(pose_a) - GetPose(pose_b));
}

static bool MTDBoxBox			(CONTACT_PARAMS) {
	BoxShape* box_a = dynamic_cast<BoxShape*>(shape_a);
	BoxShape* box_b = dynamic_cast<BoxShape*>(shape_b);

	return PenetrationOBBOBB(output,
							 BOX_PARAMS(box_a), GetPoseRot_PARAMS(pose_a),
						     BOX_PARAMS(box_b), GetPoseRot_PARAMS(pose_b));
}

static bool MTDBoxPlane			(CONTACT_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool MTDBoxConvexHull	(CONTACT_PARAMS) {
	BoxShape*    box    = dynamic_cast<BoxShape*>(shape_a);
	ConvexShape* convex = dynamic_cast<ConvexShape*>(shape_b);

	ConvexBox		box_hull    = ConvexBox(*box, GetPoseRot_PARAMS(pose_a));
	ConvexHullMesh	convex_hull = ConvexHullMesh(*convex, GetPoseRot_PARAMS(pose_b));

	return PenetrationGjkEpa(output, box_hull, convex_hull, GetPose(pose_a) - GetPose(pose_b));
}

static bool MTDPlanePlane		(CONTACT_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool MTDPlaneConvexHull	(CONTACT_PARAMS) {
	assert(false, "No Implementation");
	return false;
}

static bool MTDConvexHullConvexHull(CONTACT_PARAMS) {
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
 {Sphere,		MTDSphereSphere},
 {Capsule,		MTDSphereCapsule},
 {Box,			MTDSphereBox},
 {Plane,		MTDSpherePlane},
 {ConvexHull,	MTDSphereConvexHull}
};

static std::map<ShapeEnum, MTDFunc> capsule_mtd_funcs = {
 {Sphere,		MTDSphereCapsule},
 {Capsule,		MTDCapsuleCapsule},
 {Box,			MTDCapsuleBox},
 {Plane,		MTDCapsulePlane},
 {ConvexHull,	MTDCapsuleConvexHull}
};

static std::map<ShapeEnum, MTDFunc> box_mtd_funcs	 = {
 {Sphere,		MTDSphereBox},
 {Capsule,		MTDCapsuleBox},
 {Box,			MTDBoxBox},
 {Plane,		MTDBoxPlane},
 {ConvexHull,	MTDBoxConvexHull}
};

static std::map<ShapeEnum, MTDFunc> plane_mtd_funcs = {
{Sphere,		MTDSpherePlane},
{Capsule,		MTDCapsulePlane},
{Box,			MTDBoxPlane},
{Plane,			MTDPlanePlane},
{ConvexHull,	MTDPlaneConvexHull}
};

static std::map<ShapeEnum, MTDFunc> convex_mtd_funcs = {
{Sphere,		MTDSphereConvexHull},
{Capsule,		MTDCapsuleConvexHull},
{Box,			MTDBoxConvexHull},
{Plane,			MTDPlaneConvexHull},
{ConvexHull,	MTDConvexHullConvexHull}
};

std::map<ShapeEnum, std::map<ShapeEnum, MTDFunc>>
CollisionPenetration::mtd_funcs = {
{Sphere,		sphere_mtd_funcs},
{Capsule,		capsule_mtd_funcs},
{Box,			box_mtd_funcs},
{Plane,			plane_mtd_funcs},
{ConvexHull,	convex_mtd_funcs}
};

bool GComponent::CollisionPenetration::Penetration(PenetrationOutput& mtd, CRefShapePtrs shapes_a, CRefTransform pose_a, CRefShapePtrs shapes_b, CRefTransform pose_b)
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
					mtd.normal = -mtd.normal;
					return true;
				}
			}
		}
	}
	return false;
}

}