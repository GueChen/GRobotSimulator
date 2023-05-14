#include "system/collisionsystem.h"

#include "geometry/bounding_volume_hierarchy.h"
#include "geometry/intersection.h"
#include "geometry/compute_penetration.h"

#include "model/model.h"
#include "component/transform_component.h"
#include "component/collider_component.h"

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

void CollisionSystem::AddProcessShapes(Model* key, CRefShapePtrs shapes, CRefTransform pose, bool is_static)
{
	shape_table_[key] = std::make_tuple(shapes, pose, is_static);	
}

void CollisionSystem::AddBroadPhaseQuery(Model* key, const BoundingBox& box, int group)
{
	broad_need_process_.emplace_back(key, box, group);
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
		auto&& [shape_a, pose_a, is_static_a] = shape_table_[obj_a];
		auto&& [shape_b, pose_b, is_static_b] = shape_table_[obj_b];
		if (OverlapCheck(shape_a, pose_a, shape_b, pose_b)) {
			obj_a->intesection_ = true;
			obj_b->intesection_ = true;
			overlap_pair.emplace_back(obj_a, obj_b);
		}
	}
	
	for (auto& [obj_a, obj_b] : narrow_need_process_) {
		auto&& [shape_a, pose_a, is_static_a] = shape_table_[obj_a];
		auto&& [shape_b, pose_b, is_static_b] = shape_table_[obj_b];
		
		// ComputePenetration(shae_a, pose_a, shape_b, pose_b)
		PenetrationOutput output;
		if (CollisionPenetration::Penetration(output, shape_a, pose_a, shape_b, pose_b)) {
			/*std::cout << std::format("collision pair <{:<20}, {:<20}>:\n", obj_a->getName(), obj_b->getName());
			std::cout << "closest on A : " << output.closest_a.transpose() << std::endl;
			std::cout << "closest on B : " << output.closest_b.transpose() << std::endl;
			std::cout << "depth dir    : " << output.normal.transpose()    << std::endl;
			std::cout << "depth        : " << output.depth                 << std::endl;*/
			if (is_static_a && is_static_b) {
				// do nothing
			}
			else if (is_static_a) {
				auto trans_b = obj_b->GetTransform()->GetTransGlobal() + 1.0f * output.depth * output.normal;
				obj_b->GetTransform()->SetTransLocal(trans_b);
			}
			else if (is_static_b) {
				auto trans_a = obj_a->GetTransform()->GetTransGlobal() - 1.0f * output.depth * output.normal;
				obj_a->GetTransform()->SetTransLocal(trans_a);
			}
			else {
				auto trans_a = obj_a->GetTransform()->GetTransGlobal() - 0.5f * output.depth * output.normal;
				obj_a->GetTransform()->SetTransLocal(trans_a);
				auto trans_b = obj_b->GetTransform()->GetTransGlobal() + 0.5f * output.depth * output.normal;
				obj_b->GetTransform()->SetTransLocal(trans_b);
			}										
		}
	}

	shape_table_.clear();
	narrow_need_process_.clear();	
}

void CollisionSystem::OverlapCheck(std::vector<Vec3f>& penetration_vec, CRefShapePtrs shape, CRefTransform pose, int group)
{
	std::unordered_set<Model*> may_penetration;
	BroadPhaseQuery(may_penetration, shape, pose, group);

	for (auto& key : may_penetration) {
		auto&& [shape_a, pose_a, is_static_a] = shape_table_[key];
		PenetrationOutput output;
		if (CollisionPenetration::Penetration(output, shape_a, pose_a, shape, pose)) {
			penetration_vec.push_back(output.depth * output.normal);
		}
	}
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
		int   group;
	};

#define SAP
#ifdef SAP
	std::unordered_map<void*, std::array<std::pair<float, float>, 2>>
		sap_box_map;
	
	std::vector<SapInterval> sweep_queue;
	for (auto& [key, bounding, group] : broad_need_process_) {
		auto& part = sap_box_map[key];
		sweep_queue.emplace_back(bounding.m_min.x(), key, group);
		sweep_queue.emplace_back(bounding.m_max.x(), key, group);
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
			if (visited.count(it->key)) {
				visited.erase(it->key);
				continue;
			}
			auto cur = std::next(it);
			while (cur->key != it->key) {
				if (!visited.count(cur->key) && check_overlap(cur->key, it->key) && cur->group != it->group) {
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

void CollisionSystem::BroadPhaseQuery(std::unordered_set<Model*>& output, CRefShapePtrs shape, CRefTransform pose, int input_group)
{
	struct SapInterval {
		float point;
		void* key;
	};
	
	BoundingBox bound;
	for (auto& ptr : shape) {
		auto part_bound = BoundingBox::CompouteBoundingBox(
			*ptr, 
			pose.block(0, 3, 3, 1), 
			pose.block(0, 0, 3, 3)
		);
		bound.Merge(part_bound);
	}

	for (auto& [key, bounding, group] : broad_need_process_) {
		auto com = key->GetComponent<ColliderComponent>();
		if (input_group == group) continue;
		if (bounding.m_max.x() <= bound.m_min.x() || bound.m_max.x() <= bounding.m_min.x() ||
			bounding.m_max.y() <= bound.m_min.y() || bound.m_max.y() <= bounding.m_min.y() ||
			bounding.m_max.z() <= bound.m_min.z() || bound.m_max.z() <= bounding.m_min.z()) {
			continue;
		}
		
		output.insert(key);		
	}	
}



}