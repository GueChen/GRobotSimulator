/**
 *  @file  	bounding_box.h
 *  @brief 	this class supply a simple implementation about bounding box structure.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 25th, 2023
 **/
#ifndef __GBOUNDING_BOX_H
#define __GBOUNDING_BOX_H

#include "geometry/geometry_datastructure.hpp"
#include "geometry/abstract_shape.hpp"

#include <Eigen/Core>

#include <numeric>
#include <memory>

namespace GComponent {

using Vec3f = Eigen::Vector3f;
using SO3f  = Eigen::Matrix3f;
struct BoundingBox {
	BoundingBox()  = default;
	BoundingBox(const Vec3f& min, const Vec3f& max): 
		m_min(min), m_max(max) {}
	~BoundingBox() = default;

	BoundingBox(const BoundingBox& other) = default;
	BoundingBox(BoundingBox&& other)	  = default;

	BoundingBox& operator=(const BoundingBox& other) = default;
	BoundingBox& operator=(BoundingBox&& other)		 = default;

/// Static Methods
	static BoundingBox MergeTwoBoundingBox(const BoundingBox& box_a, const BoundingBox& box_b);
	static BoundingBox CompouteBoundingBox(const AbstractShape& shape, const Vec3f& trans, const SO3f& rot);

/// Fields
	Vec3f m_min = Vec3f::Ones() * std::numeric_limits<float>::max();
	Vec3f m_max = Vec3f::Ones() * std::numeric_limits<float>::lowest();
};




}

#endif // !__GBOUNDING_BOX_H
