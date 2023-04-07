#include "compute_penetration.h"

namespace GComponent{



bool GComponent::CollisionPenetration::Penetration(Vec3f& mtd, CRefShapePtrs shapes_a, CRefTransform pose_a, CRefShapePtrs shapes_b, CRefTransform pose_b)
{
	for (auto shape_a : shapes_a) {
		ShapeEnum type_a = shape_a->GetShapeType();
		for (auto shape_b : shapes_b) {
			auto& mtd_func = mtd_funcs[type_a][shape_b->GetShapeType()];
			if (type_a > shape_b->GetShapeType()) {
				if (mtd_func(mtd, shape_a, pose_a, shape_b, pose_b)) {
					return true;
				}
			}
			else {
				if (mtd_func(mtd, shape_b, pose_b, shape_a, pose_a)) {
					return true;
				}
			}
		}
	}
	return false;
}

}