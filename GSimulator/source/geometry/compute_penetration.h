/**
 *  @file  	compute_penetration.h
 *  @brief 	add some description.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Apr 7th, 2022
 **/
#ifndef __G_COMPUTE_PENETRATION_H
#define __G_COMPUTE_PENETRATION_H

#include "geometry/abstract_shape.hpp"

#include <functional>
#include <map>

namespace GComponent {

using Vec3f	  = Eigen::Vector3f;
// Minimal Translation Distance(MTD) function table
using MTDFunc = std::function<bool(Vec3f&,
								   AbstractShape*, CRefTransform,
								   AbstractShape*, CRefTransform)>;

class CollisionPenetration {
public:
	static bool Penetration(Vec3f& mtd, 
						    CRefShapePtrs shapes_a, CRefTransform pose_a,
						    CRefShapePtrs shapes_b, CRefTransform pose_b);
private:
	static std::map<ShapeEnum, std::map<ShapeEnum, MTDFunc>> mtd_funcs;
};

} // !namespace GComponent

#endif // !__G_COMPUTE_PENETRATION_H
