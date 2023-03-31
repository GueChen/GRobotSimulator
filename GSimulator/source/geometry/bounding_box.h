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
	enum Dim {
		X = 0, Y, Z
	};
	BoundingBox()  = default;
	BoundingBox(const Vec3f& min, const Vec3f& max): 
		m_min(min), m_max(max) {}
	~BoundingBox() = default;

	BoundingBox(const BoundingBox& other) = default;
	BoundingBox(BoundingBox&& other)	  = default;

	BoundingBox& operator=(const BoundingBox& other) = default;
	BoundingBox& operator=(BoundingBox&& other)		 = default;

	Vec3f	Differ()	  const;
	Vec3f	Centroid()	  const;
	Dim		MaxExtent()	  const;
	float	SurfaceArea() const;

	/// <summary>
	/// return the relative value scaled by box extent using {m_min} as original point
	/// <para>
	/// 返回一个相对于盒子最小点的缩放值，缩放比例以盒子扩充轴为基准
	/// </para>
	/// </summary>
	/// <param name="p"> cref {Vec3f} 输入点 </param>
	/// <returns> cref {Vec3f} 输出比值点，若点在盒子内，则取值范围为 [0, 1], 盒子外则超出该范围 </returns>
	Vec3f	RelativeScale(const Vec3f& p) const;

	BoundingBox& Merge(const BoundingBox& other);
	BoundingBox& Merge(const Vec3f& p);

/// Static Methods
	static BoundingBox MergeTwoBoundingBox(const BoundingBox& box_a, const BoundingBox& box_b);
	static BoundingBox MergeWithPoint	  (const BoundingBox& box,	 const Vec3f& p);
	static BoundingBox CompouteBoundingBox(const AbstractShape& shape, const Vec3f& trans, const SO3f& rot);
/// Fields
	Vec3f m_min = Vec3f::Ones() * std::numeric_limits<float>::max();
	Vec3f m_max = Vec3f::Ones() * std::numeric_limits<float>::lowest();
};




}

#endif // !__GBOUNDING_BOX_H
