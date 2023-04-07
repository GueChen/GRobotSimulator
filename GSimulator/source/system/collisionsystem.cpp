#include "system/collisionsystem.h"

#include "geometry/bounding_volume_hierarchy.h"
#include "geometry/intersection.h"
#include "model/model.h"

#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace GComponent {

/*_____________________COLLISION SYSTEMS CLASS________________________________*/
/*_____________________PUBLIC METHODS_________________________________________*/
CollisionSystem& CollisionSystem::getInstance()
{
	static CollisionSystem instance;
	return instance;
}

CollisionSystem::~CollisionSystem() = default;

void CollisionSystem::Initialize()
{}

void CollisionSystem::AddProcessShapes(Model* key, CRefShapePtrs shapes, CRefTransform pose)
{
	shape_table_[key] = std::make_pair(shapes, pose);	
}

void CollisionSystem::AddBroadPhaseQuery(Model* key, const BoundingBox& box)
{
	broad_need_process_.push_back(std::make_pair(key, box));
}

void CollisionSystem::tick(float delta_time)
{
	(void)delta_time;
	// Broad Phase Query
	BroadPhaseQuery();

	std::vector<std::pair<Model*, Model*>> overlap_pair;
	//TODO: add broad phase collision checking to accelerate the whole process
	// Narrow Phase Overlap Test
	for (auto& [obj_a, obj_b] : narrow_need_process_) {
		auto&& [shape_a, pose_a] = shape_table_[obj_a];
		auto&& [shape_b, pose_b] = shape_table_[obj_b];
		if (OverlapCheck(shape_a, pose_a, shape_b, pose_b)) {
			obj_a->intesection_ = true;
			obj_b->intesection_ = true;
			overlap_pair.emplace_back(obj_a, obj_b);
		}
	}
	
	for (auto& [obj_a, obj_b] : narrow_need_process_) {
		auto&& [shape_a, pose_a] = shape_table_[obj_a];
		auto&& [shape_b, pose_b] = shape_table_[obj_b];
		
		// ComputePenetration(shae_a, pose_a, shape_b, pose_b)

	}

	shape_table_.clear();
	narrow_need_process_.clear();	
}

bool CollisionSystem::OverlapCheck(CRefShapePtrs shapes_a, Transform pose_a,
								   CRefShapePtrs shapes_b, Transform pose_b)
{
	return Intersection::IntersectCheck(shapes_a, pose_a, shapes_b, pose_b);
}

void CollisionSystem::BroadPhaseQuery()
{
	// sweep-and-prune
	struct SapInterval {		
		float point;		
		void* key;
	};

#define SAP
#ifdef SAP
	std::unordered_map<void*, std::array<std::pair<float, float>, 2>>
		sap_box_map;
	
	std::vector<SapInterval> sweep_queue;
	for (auto& [key, bounding] : broad_need_process_) {
		auto& part = sap_box_map[key];
		sweep_queue.emplace_back(bounding.m_min.x(), key);
		sweep_queue.emplace_back(bounding.m_max.x(), key);
		part[0] = { bounding.m_min.y(), bounding.m_max.y()};
		part[1] = { bounding.m_min.z(), bounding.m_max.z()};		
	}

	std::sort(sweep_queue.begin(), sweep_queue.end(), [](auto&& i1, auto&& i2) {
		return i1.point < i2.point;
		});
	
	auto check_overlap = [&map =sap_box_map](auto key1, auto key2) {
		auto& col1 = map[key1], &col2 = map[key2];
		return !(col1[0].second < col2[0].first || col2[0].second < col1[0].first) &&
			   !(col1[1].second < col2[1].first || col2[1].second < col1[1].first);
	};	
	
	{
		std::unordered_set<void*> visited;

		for(auto it = sweep_queue.begin(); it != sweep_queue.end();++it){
			if (visited.count(it->key)) continue;
			auto cur = std::next(it);
			while (cur->key != it->key) {				
				if (!visited.count(cur->key) && check_overlap(cur->key, it->key)) {
					narrow_need_process_.emplace_back((Model*)it->key, (Model*)cur->key);					
				}
				cur = std::next(cur);
			}
			visited.insert(it->key);
		}
	}
#endif
	broad_need_process_.clear();
}



}