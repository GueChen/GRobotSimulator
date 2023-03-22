#include "geometry/convex_wrapper/convex_box.h"

namespace GComponent{

Vec3f ConvexBox::Support(const Vec3f& dir) const
{
	const Vec3f local_dir = rot_.transpose() * dir;
	Vec3f		max_vertex(shape_data_.m_half_x, shape_data_.m_half_y, shape_data_.m_half_z);
	for (int i = 0; i < 3; ++i) if (local_dir(i) <= 0) { 
		max_vertex(i) *= -1.0f;
	}
	return rot_ * max_vertex + center_;
}

}