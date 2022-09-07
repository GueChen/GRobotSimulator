#include "manager/planningmanager.h"
#include "system/planningsystem.h"
#include "motion/GMotion"

#include "component/joint_group_component.h"
#include <QtCore/QThreadPool>

#include <chrono>

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG


namespace GComponent{

PlanningManager& PlanningManager::getInstance()
{
	static PlanningManager instance;
	return instance;	
}

PlanningManager::~PlanningManager() = default;

void PlanningManager::RegisterPlanningTask(Model * robot, JTrajFunc func, float time_total, uint32_t interval)
{	
	auto task = [func, time_total, interval, m_key = robot, &m_locks = lock_map, &m_que = task_queue_map]() {
		if (isinf(time_total) || isnan(time_total)) {
#ifdef _DEBUG
			std::cout << "Time Is Infinity or nan val, Speed Divide Zero!\n";
#endif
			return;
		}

		{													// RAII scope
		std::lock_guard<std::mutex> lock(m_locks[m_key]);	// guaranted only one task possessing the ownership of robot

		float elapsed = 0.0f;
		std::chrono::time_point start_time = std::chrono::steady_clock::now();
		while (elapsed < time_total) {
			elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - start_time).count();
			auto j_pairs = func(elapsed);
			if (!j_pairs.first.empty()) {
				vector<float>& joints = j_pairs.first;				
				JointGroupComponent* jsdk = m_key->GetComponent<JointGroupComponent>(JointGroupComponent::type_name.data());
				if(jsdk->SafetyCheck(joints)){
					PlanningSystem::getInstance().BroadcastJointsAngle(m_key->getName(), joints);
				}
#ifdef _DEBUG
				else {
					std::cout << "Safety check failed for limitation resaon\n";
				}
#endif // _DEBUG
			}

#ifdef _DEBUG
			std::cout << "_____________________________________\n";
			std::cout << "elapsed: " << elapsed << std::endl;
			for (int i = 0; i < j_pairs.first.size(); ++i) {
				std::cout << std::format("x val:{: >10}, v val:{: >10}\n", j_pairs.first[i], j_pairs.second[i]);
			}
			std::cout << "_____________________________________\n";
#endif
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
				
		}
		}													// !RAII scope
		if (m_que.find(m_key) != m_que.end()) {			    // no task anymore, delete mutex from map 
			if (m_que[m_key].empty()) {
				m_locks.erase(m_key);
			}
			else {
				QThreadPool::globalInstance()->start(m_que[m_key].front());
				m_que[m_key].pop();
				if (m_que[m_key].empty()) {
					m_que.erase(m_key);
				}
			}
		}
		
	};
	std::mutex& lock = lock_map[robot];
	if (lock.try_lock()) {		
		QThreadPool::globalInstance()->start(task);
		lock.unlock();
	}
	else {
		task_queue_map[robot].push(task);
	}
		
}

void PlanningManager::RegisterDualPlanningTask(std::vector<Model*> robots, std::vector<JTrajFunc> func, float time_total, uint32_t interval_ms)
{
	auto task = [funcs = func, time_total, interval = interval_ms, m_keys = robots, &m_locks = lock_map, &m_que = task_queue_map] {
		{
		std::lock_guard<std::mutex> lock_l(m_locks[m_keys[0]]),
									lock_r(m_locks[m_keys[1]]);
		float elapsed = 0.0f;
		std::chrono::time_point start_time = std::chrono::steady_clock::now();
		
		while (elapsed < time_total) {
			elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - start_time).count();
			auto l = funcs[0](elapsed);
			auto r = funcs[1](elapsed);

			if (!l.first.empty()) {								
				PlanningSystem::getInstance().BroadcastJointsAngle(m_keys[0]->getName(), l.first);
			}
			if (!r.first.empty()) {								
				PlanningSystem::getInstance().BroadcastJointsAngle(m_keys[1]->getName(), r.first);
			}
#ifdef  _DEBUG
			std::cout << "_____________________________________\n";
			std::cout << "elapsed: " << elapsed << std::endl;

			for (int i = 0; i < l.first.size(); ++i) {
				std::cout << std::format("x val:{: >10}, v val:{: >10}\n", l.first[i], l.second[i]);
			}
			for (int i = 0; i < r.first.size(); ++i) {
				std::cout << std::format("x val:{: >10}, v val:{: >10}\n", r.first[i], r.second[i]);
			}
			std::cout << "_____________________________________\n";
#endif //  _DEBUG
			
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));				
		}
		}		
	};

	QThreadPool::globalInstance()->start(task);
	// consider how to process the concurrency for dual situation
	// std::cout << "RegisterDualPlanningTask NO IMPLEMENTATIONS\n";
}

void PlanningManager::tick(float delta_time)
{

}

}