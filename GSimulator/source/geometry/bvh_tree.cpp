#include "geometry/bounding_volume_hierarchy.h"

#include <algorithm>
#include <stack>
namespace GComponent {

static int EqualSplit(std::vector<BVHInfo>& eles, int start, int end, int dim) {
	int mid = (start + end) / 2;
	std::nth_element(std::next(eles.begin(), start), std::next(eles.begin(), mid), std::next(eles.begin(), end),
		[dim = dim](const BVHInfo& i1, const BVHInfo& i2) {								// use infos[mid] to divide bounds into 2 parts
			return i1.m_centroid(dim) < i2.m_centroid(dim);
		});
	return mid;
}

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
	// Current Node 
	BoundingBox bounds;
	for (int i = start; i < end; ++i) {
		bounds.Merge(infos[i].m_bounds);
	}

	int nb_bound = end - start;

	// push all bounds elements into ordered bounds array
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
				mid = EqualSplit(infos, start, end, dim);
				break;
			}
			case SplitMethod::SAH: {
				if (nb_bound <= 4) {
					mid = EqualSplit(infos, start, end, dim);
				}
				else {
					constexpr int nb_buckets = 12;
					struct BucketInfo {
						int			m_count = 0;
						BoundingBox m_bounds;
					};
					BucketInfo buckets[nb_buckets];
					
					// split all bounding box into bucket according to its relative scale coordinate of centroid 
					// from centorid bounding box minimum corner point 
					for (int i = start; i < end; ++i) {
						int buck_idx = nb_bound * centroid_bound.RelativeScale(infos[i].m_centroid)(dim);
						buck_idx = std::min(buck_idx, nb_bound - 1);						
						buckets[buck_idx].m_bounds.Merge(infos[i].m_bounds);
						++buckets[buck_idx].m_count;
					}

					// O(n^2) operation to caculate all possible split method costs
					float buckets_costs[nb_buckets];
					for (int i = 0; i < nb_buckets - 1; ++i) {
						BoundingBox b0, b1;
						int count0 = 0, count1 = 0;
						for (int j = 0; j <= i; ++j) {
							b0.Merge(buckets[j].m_bounds);
							count0 += buckets[j].m_count;
						}
						for (int j = i + 1; j < nb_buckets; ++j) {
							b1.Merge(buckets[j].m_bounds);
							count1 += buckets[j].m_count;
						}
						buckets_costs[i] = 0.125f + (count0 * b0.SurfaceArea() + count1 * b1.SurfaceArea()) / bounds.SurfaceArea();					
					}

					// traversal to find which split cost is the minimum cost
					float min_cost = buckets_costs[0];
					int   min_cost_split_bucket = 0;
					for (int i = 1; i < nb_bound - 1; ++i) {
						if (buckets_costs[i] < min_cost) {
							min_cost = buckets_costs[i];
							min_cost_split_bucket = i;
						}
					}

					float leaf_cost = nb_bound;
					if (min_cost < leaf_cost) {
						// using min cost split idx to split bounds into 2 parts acoording to bucket idx						
						auto mid_iter = std::partition(
							std::next(infos.begin(), start), std::next(infos.begin(), end),
							[max_idx = nb_bound - 1, dim = dim, scale_bound = centroid_bound, split_idx = min_cost_split_bucket]
							(const BVHInfo& i) {
								int buck_idx = nb_buckets * scale_bound.RelativeScale(i.m_centroid)(dim);
								buck_idx = std::min(buck_idx, max_idx);
								return buck_idx <= split_idx;
							}
						);
					}
					else {
						create_leaf_node();
					}
				}
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