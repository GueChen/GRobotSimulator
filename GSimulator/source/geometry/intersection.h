/**
 *  @file  	intersection.h
 *  @brief 	this file supply intesection test function.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	April 7th, 2022
 **/
#ifndef __G_INTERSECTION_H
#define __G_INTERSECTION_H

#include "geometry/abstract_shape.hpp"

#include "base/singleton.h"

#include <map>

namespace GComponent {
class Intersection{
	Intersection() = delete;
	NonCopyable(Intersection);

public:
using IntersectionFunc = std::function<bool(AbstractShape*, CRefTransform,
											AbstractShape*, CRefTransform)>;

static bool IntersectCheck(CRefShapePtrs shapes_a, CRefTransform pose_a,
						   CRefShapePtrs shapes_b, CRefTransform pose_b);

private:
static std::map<ShapeEnum, std::map<ShapeEnum, IntersectionFunc>> intersection_funcs;
};
} // !namespace GComponent

#endif // !__G_INTERSECTION_H
