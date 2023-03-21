#include "geometry/convex_wrapper/convex_hull.h"

namespace GComponent {

Vec3f ConvexHullMesh::Support(const Vec3f& dir) const
{
	const Eigen::Vector3f local_dir = rot_.transpose() * dir;
	const Eigen::Vector3f max_vertex = SupportLocal(local_dir);
	return rot_ * max_vertex + center_;
}

Vec3f ConvexHullMesh::SupportLocal(const Vec3f& local_dir) const
{
	// brute force search
	// TODO : optimize using physx hill climbing searching
	const std::vector<Vec3f>& poses = shape_data_.m_positions;

	float max_dot = poses[0].dot(local_dir);
	int   max_idx = 0;
	for (int i = 1; i < poses.size(); ++i) {
		const float dot_val = poses[i].dot(local_dir);
		if (dot_val > max_dot) {
			max_dot = dot_val;
			max_idx = i;
		}
	}
	return poses[max_idx];
}

}