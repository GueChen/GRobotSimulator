#include "geometry/convex_wrapper/convex_sphere.h"

namespace GComponent{

Vec3f ConvexSphere::Support(const Vec3f& dir) const
{
	return center_ + dir * shape_data_.m_radius;
}

}