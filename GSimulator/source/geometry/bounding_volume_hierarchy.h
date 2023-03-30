/**
 *  @file  	bounding_volume_hierarchy.h
 *  @brief 	This file supply a speed-up datastructure for collision detect in broad phase.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 27th, 2023
 **/
#ifndef __GBOUNDING_VOLUME_HIERARCHY_H
#define __GBOUNDING_VOLUME_HIERARCHY_H

#include "geometry/bounding_box.h"

#include <Eigen/Core>

#include <vector>
#include <memory>

namespace GComponent {


using Vec3f = Eigen::Vector3f;

struct BVHNode {
/// Static Methods
	static BVHNode CreateLeaf(int first, int n, const BoundingBox& b);

/// Methods
	void InsertChildren(int axis, BVHNode* child0, BVHNode* child1);

/// Fields Êý¾ÝÓò
	BoundingBox bound_;
	BVHNode*	children_[2] = {nullptr};
	int			axis_;
	int			first_offset_;
	int			count_;
};

struct BVHInfo {
	BVHInfo(size_t number, const BoundingBox& bounds) : 
		m_number(number), m_bounds(bounds), m_centroid(bounds.Centroid())
	{}

	size_t		m_number;
	BoundingBox m_bounds;
	Vec3f		m_centroid;
};

struct LinearBVHNode {
	BoundingBox m_bounds;
	union {
		int m_bound_offset;
		int m_second_child_offset;
	};
	uint16_t	m_nb_bounds;
	uint8_t     m_axis;
	uint8_t     m_pad[1];
};

class BVHTree {
public:
	enum SplitMethod {
		SAH, HLBVH, Middle, EqualCounts
	};

	BVHTree(const std::vector<BoundingBox>& basics_, SplitMethod split_method);
	~BVHTree();
	std::vector<BoundingBox> GetBoundings() const;

protected:
	BVHNode* RecursiveBuild(int& tot, std::vector<BoundingBox>& ordered_bounds, std::vector<BVHInfo>& infos, int start, int end);
	int		 FlattenBVHTree(BVHNode* node, int& offset);

private:
	SplitMethod					split_;
	std::vector<BoundingBox>    bounds_;
	std::vector<LinearBVHNode>	nodes_;
	int							nb_nodes_ = 0;
};
using GBVH = BVHTree;

} // !namespace GComponent

#endif // !__GBOUNDING_VOLUME_HIERARCHY_H
