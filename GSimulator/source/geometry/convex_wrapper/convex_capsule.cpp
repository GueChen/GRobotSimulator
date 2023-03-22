#include "geometry/convex_wrapper/convex_capsule.h"

namespace GComponent {

Vec3f ConvexCapsule::Support(const Vec3f& dir) const
{
	const Vec3f local_dir = rot_.transpose() * dir;
	Vec3f p = shape_data_.m_half_height * Vec3f::UnitZ();
	Vec3f max_vertex = (local_dir.z() >= 0 ? p : -p) + 
						shape_data_.m_radius * local_dir;
	return rot_ * max_vertex + center_;
}
}