#include "geometry/bounding_volume_hierarchy.h"

#include <algorithm>
#include <stack>
namespace GComponent {
BVHTree::BVHTree(const std::vector<BoundingBox>& bounds, SplitMethod split_method):
	bounds_(bounds), 
	split_(split_method)	// FIXME: is rebundunt ?
{
	if (bounds.size() == 0) return;
	std::vector<BVHInfo> info; info.reserve(bounds.size());
	for (size_t i = 0; auto & bound : bounds) {
		info.emplace_back(i++, bound);
	}

	std::vector<BoundingBox> ordered_bounds;
	int nb_nodes = 0;
	BVHNode* root;
	if (split_method == HLBVH) {
		assert(false && "HLBVH No Implementation\n");
		//root = HLBVHBuild();
	}
	else {
		root = RecursiveBuild(nb_nodes, ordered_bounds, info, 0, bounds.size());
	}

	bounds_.swap(ordered_bounds);

	nodes_.resize(nb_nodes);
	
	int offset = 0;
	FlattenBVHTree(root, offset);
	nb_nodes_ = nb_nodes;
	
}

BVHTree::~BVHTree()
{

}

std::vector<BoundingBox> BVHTree::GetBoundings() const
{
	std::vector<BoundingBox> ret;	
	for (int i = 0; i < nb_nodes_; ++i) {
		ret.push_back(nodes_[i].m_bounds);
	}
	return ret;
}

BVHNode* BVHTree::RecursiveBuild(int& tot, std::vector<BoundingBox>& ordered_bounds, std::vector<BVHInfo>& infos, int start, int end)
{
	BVHNode* node = new BVHNode;
	++tot;
	BoundingBox bounds;
	for (int i = start; i < end; ++i) {
		bounds.Merge(infos[i].m_bounds);
	}

	int nb_bound = end - start;
	auto create_leaf_node = [&]() {
		int first_offset = ordered_bounds.size();
		for (int i = start; i < end; ++i) {
			ordered_bounds.push_back(bounds_[infos[i].m_number]);
		}
		*node = BVHNode::CreateLeaf(first_offset, nb_bound, bounds);
	};
	
	if (nb_bound == 1) {
		create_leaf_node();
	}
	else {
		BoundingBox centroid_bound;
		for (int i = start; i < end; ++i) {
			centroid_bound.Merge(infos[i].m_centroid);
		}
		BoundingBox::Dim dim = centroid_bound.MaxExtent();
		int mid = (start + end) / 2;
		if (centroid_bound.m_max(dim) == centroid_bound.m_min(dim)) {
			create_leaf_node();
		}
		else {
			switch (split_) {
			case SplitMethod::Middle: {
				float mid_point = 0.5f * (centroid_bound.m_min(dim) + centroid_bound.m_max(dim));	// using mid point split infos into 2 parts
				auto  mid_iter = std::partition(std::next(infos.begin(), start), std::next(infos.begin(), end),																									
					[dim = dim, mid_point = mid_point](const BVHInfo& info){						// quick select 
						return info.m_centroid(dim) < mid_point;
					}
				);

				mid = std::distance(infos.begin(), mid_iter);
				assert(start <= mid && mid < end && "mid must between [s, e)");
				if (mid != start && mid != end) break;												// one equal means not divide infos into 2 different parts				
			}
			case SplitMethod::EqualCounts: {
				mid = (start + end) / 2;
				std::nth_element(std::next(infos.begin(), start), std::next(infos.begin(), mid), std::next(infos.begin(), end),
					[dim = dim](const BVHInfo& i1, const BVHInfo& i2) {								// use infos[mid] to divide bounds into 2 parts
						return i1.m_centroid(dim) < i2.m_centroid(dim);
					});
				break;
			}
			case SplitMethod::SAH: {
				assert(false && "SAH No Implementation\n");
				break;
			}
			}
			assert(start <= mid && mid < end && "mid must between [s, e)");
			node->InsertChildren(dim, 
								 RecursiveBuild(tot, ordered_bounds, infos, start, mid),
								 RecursiveBuild(tot, ordered_bounds, infos, mid, end));
		}
	}
	return node;
}

int BVHTree::FlattenBVHTree(BVHNode* node, int& offset)
{
	LinearBVHNode* linear_node = &nodes_[offset];
	linear_node->m_bounds = node->bound_;
	int ret_offset = offset++;
	if (node->count_ > 0) {
		linear_node->m_bound_offset = node->first_offset_;
		linear_node->m_nb_bounds    = node->count_;
	}
	else {
		linear_node->m_axis = node->axis_;
		linear_node->m_nb_bounds = 0;
		FlattenBVHTree(node->children_[0], offset);
		linear_node->m_second_child_offset = FlattenBVHTree(node->children_[1], offset);
	}
	free(node);
	return ret_offset;
}
}