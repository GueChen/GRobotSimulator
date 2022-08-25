#include "manager/planningmanager.h"

#include "component/joint_group_component.h"
#include "motion/GMotion"

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
		if (isinf(time_total)) {
#ifdef _DEBUG
			std::cout << "Time Is Infinity, Speed Divide Zero!\n";
#endif
			return;
		}

		{													// RAII scope
		std::lock_guard<std::mutex> lock(m_locks[m_key]);	// guaranted there is only one task posses the ownership of robot

		float elapsed = 0.0f;
		std::chrono::time_point start_time = std::chrono::steady_clock::now();
		while (elapsed < time_total) {
			elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - start_time).count();
			auto j_pairs = func(elapsed);

			if (!j_pairs.empty()) {
				JointGroupComponent& joints_sdk = *m_key->GetComponent<JointGroupComponent>(JointGroupComponent::type_name.data());
				vector<float> joints(j_pairs.size());
				std::transform(j_pairs.begin(), j_pairs.end(), joints.begin(), [](auto&& p) {return p.first; });
				joints_sdk.SetPositions(joints);
			}

#ifdef _DEBUG
			std::cout << "_____________________________________\n";
			std::cout << "elapsed: " << elapsed << std::endl;
			for (auto& [x, v] : j_pairs) {
				std::cout << std::format("x val:{: >10}, v val:{: >10}\n", x, v);
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

			if (!l.empty()) {
				JointGroupComponent& joints_sdk = *m_keys[0]->GetComponent<JointGroupComponent>(JointGroupComponent::type_name.data());
				vector<float> joints(l.size());
				std::transform(l.begin(), l.end(), joints.begin(), [](auto&& p) {return p.first; });
				joints_sdk.SetPositions(joints);
			}
			if (!r.empty()) {
				JointGroupComponent& joints_sdk = *m_keys[1]->GetComponent<JointGroupComponent>(JointGroupComponent::type_name.data());
				vector<float> joints(r.size());
				std::transform(r.begin(), r.end(), joints.begin(), [](auto&& p) {return p.first; });
				joints_sdk.SetPositions(joints);
			}
#ifdef  _DEBUG
			std::cout << "_____________________________________\n";
			std::cout << "elapsed: " << elapsed << std::endl;

			for (auto& [x, v] : l) {
				std::cout << std::format("x val:{: >10}, v val:{: >10}\n", x, v);
			}
			for (auto& [x, v] : r) {
				std::cout << std::format("x val:{: >10}, v val:{: >10}\n", x, v);
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

}