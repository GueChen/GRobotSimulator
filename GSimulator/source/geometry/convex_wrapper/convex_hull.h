/**
 *  @file  	convex_hull.h
 *  @brief 	This class supply a interface to realize support function.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 20th, 2023
 **/
#ifndef __GCONVEX_HULL_H
#define __GCONVEX_HULL_H

#include "GComponent/Geometry/gjk_convex.h"

#include "geometry/geometry_datastructure.hpp"
#include "geometry/abstract_shape.hpp"

#include <memory>

namespace GComponent {
// TODO: Not Considering the Scale situations
using Vec3f = Eigen::Vector3f;
using SO3f  = Eigen::Matrix3f;
class ConvexHullMesh : public GJKConvex {
public:
	ConvexHullMesh(ConvexShape& input, Vec3f trans, SO3f rot):
		shape_data_(input),
		center_	   (std::move(trans)),
		rot_	   (std::move(rot))
	{}

	Vec3f Support(const Vec3f& dir) const override;

private:
	Vec3f SupportLocal(const Vec3f& local_dir) const;

private:
	ConvexShape& shape_data_;
	Vec3f		 center_;
	SO3f		 rot_;


};
}

#endif // !__GCONVEX_HULL_H
