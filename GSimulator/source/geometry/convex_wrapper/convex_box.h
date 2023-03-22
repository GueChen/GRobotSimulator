/**
 *  @file  	convex_box.h
 *  @brief 	This class supply a interface to realize box shape support function.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 22th, 2023
 **/
#ifndef __GCONVEX_BOX_H
#define __GCONVEX_BOX_H

#include "geometry/geometry_datastructure.hpp"
#include "geometry/abstract_shape.hpp"

#include <memory>

namespace GComponent {

using Vec3f = Eigen::Vector3f;
using SO3f  = Eigen::Matrix3f;
class ConvexBox {
public:
	ConvexBox(BoxShape& input, Vec3f trans, SO3f rot):
		shape_data_(input),
		center_	   (std::move(trans)),
		rot_	   (std::move(rot))
	{}

	Vec3f Support(const Vec3f&) const;

private:
	BoxShape& shape_data_;
	Vec3f	  center_;
	SO3f	  rot_;
};


}


#endif // !__GCONVEX_BOX_H
