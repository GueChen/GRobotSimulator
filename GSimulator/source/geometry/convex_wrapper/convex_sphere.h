/**
 *  @file  	convex_sphere.h
 *  @brief 	This Class is a wrapper class used for sphere shape supply support function.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 20th, 2023
 **/
#ifndef __GCONVEX_SPHERE_H
#define __GCONVEX_SPHERE_H

#include "GComponent/Geometry/gjk_convex.h"

#include "geometry/abstract_shape.hpp"

#include <memory>

namespace GComponent{

using Vec3f = Eigen::Vector3f;
using SO3f  = Eigen::Matrix3f;

class ConvexSphere : public GJKConvex{
public:
	ConvexSphere(SphereShape& input, Vec3f trans, SO3f rot):
		shape_data_(input),
		center_	   (std::move(trans)),
		rot_	   (std::move(rot))
	{}

	Vec3f Support(const Vec3f& dir) const override;

private:
	SphereShape& shape_data_;
	Vec3f		 center_;
	SO3f		 rot_;
};

}
#endif // !__GCONVEX_SPHERE_H
