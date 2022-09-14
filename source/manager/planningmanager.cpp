#include "manager/planningmanager.h"
#include "system/planningsystem.h"
#include "system/skinsystem.h"
#include "motion/GMotion"

#include "component/joint_group_component.h"
#include <QtCore/QThreadPool>

#include <chrono>

#define _DEBUG
#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG

#ifdef _DEBUG
void DebugPrintMessage(float elapsed, const GComponent::JointPairs& js) {
	std::cout << "_____________________________________\n";
	std::cout << "elapsed: " << elapsed << std::endl;
	for (int i = 0; i < js.first.size(); ++i) {
		std::cout << std::format("x val:{: >10}, v val:{: >10}\n", js.first[i], js.second[i]);
	}
	std::cout << "_____________________________________\n";
}
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
	auto task = [this, func, time_total, interval, 
				 m_key = robot,										// map key use model ptr
				 &m_locks = lock_map_,								// get lock
				 &m_que   = task_queue_map_,						// get queue 
				 &m_status= execution_flag_map_,
				 &manager_elapsed_time = this->task_time_stamp_]
		() mutable {
		using namespace std::chrono;
		if (isinf(time_total) || isnan(time_total)) {
#ifdef _DEBUG
			std::cout << "Time Is Infinity or nan val, Speed Divide Zero!\n";
#endif
			goto next_task;
		}

		{													// RAII lock scope
		std::lock_guard<std::mutex> lock(m_locks[m_key]);	// guaranted only one task possessing the ownership of robot
		
		float	    elapsed	   = 0.0f,
				    delay	   = 0.0f;
		manager_elapsed_time   = elapsed;
							   
		TaskStatus& status	   = m_status[m_key],
					last_status= status;
		time_point  start_time = steady_clock::now(),
					last_time  = steady_clock::now();

		while (elapsed < time_total && status != eStop) {
			time_point cur_time = steady_clock::now();
			elapsed  = duration_cast<duration<float>>(cur_time - start_time).count() - delay;
			if (status == eStop) goto next_task;
			if (status == ePause || last_status == ePause) {
				delay += duration_cast<duration<float>>(cur_time - last_time).count();				
			}			
			else if (status == eExecution) {
				ExecutionOnce(func, m_key, elapsed);		// normal execution
			}

			manager_elapsed_time = elapsed;
			std::this_thread::sleep_for(milliseconds(interval));
			last_status = status;
			last_time	= cur_time;
		}

		if (status == eExecution) {							// gurantee goal reached
			ExecutionOnce(func, m_key, time_total);
		}
		}													// !RAII lock scope
		
	next_task:
		TaskFinishedNotify(m_key);		
	};

	std::mutex& lock = lock_map_[robot];
	if (lock.try_lock()) {
		execution_flag_map_[robot] = eExecution;
		QThreadPool::globalInstance()->start(task);
		lock.unlock();
	}
	else {
		task_queue_map_[robot].push(task);
	}
}

void PlanningManager::RegisterDualPlanningTask(std::vector<Model*> robots, std::vector<JTrajFunc> func, float time_total, uint32_t interval_ms)
{
	auto task = [this, funcs = func, time_total, interval = interval_ms, 
				 m_keys = robots, 
				 &m_status= execution_flag_map_,
				 &m_locks = lock_map_, 
				 &m_que = task_queue_map_] {
		using namespace std::chrono;
		{
		std::lock_guard<std::mutex> lock_l(m_locks[m_keys[0]]),
									lock_r(m_locks[m_keys[1]]);

		float	    elapsed	   = 0.0f,
				    delay	   = 0.0f;
		//manager_elapsed_time   = elapsed;

		TaskStatus& l_status	   = m_status[m_keys[0]],
					l_last_status  = l_status,
					r_status	   = m_status[m_keys[1]],
					r_last_status  = r_status;

		time_point  start_time = steady_clock::now(),
					last_time  = steady_clock::now();			
		
		while (elapsed < time_total && l_status) {
			elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - start_time).count();
			auto l = funcs[0](elapsed);
			auto r = funcs[1](elapsed);

			if (!l.first.empty() && !r.first.empty()) {								
				PlanningSystem::getInstance().BroadcastJointsAngle(m_keys[0]->getName(), l.first, elapsed);
				PlanningSystem::getInstance().BroadcastJointsAngle(m_keys[1]->getName(), r.first, elapsed);
			}

#ifdef  _DEBUG
			DebugPrintMessage(elapsed, l);
			DebugPrintMessage(elapsed, r);			
#endif //  _DEBUG
			
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));				
		}
		}		
	};
	
	QThreadPool::globalInstance()->start(task);

	// consider how to process the concurrency for dual situation
	// std::cout << "RegisterDualPlanningTask NO IMPLEMENTATIONS\n";
}

void PlanningManager::ChangeCurrentTaskStatus(Model* robot, int status)
{
	auto iter = execution_flag_map_.find(robot);
	if (iter == execution_flag_map_.end()) return;
	iter->second = static_cast<TaskStatus>(status);
}

void PlanningManager::tick(float delta_time)
{

}

/*___________________________________PROTECTED METHODS_____________________________*/
bool PlanningManager::ExecutionOnce(const JTrajFunc& func, Model* obj, float time) const {
	auto j_pairs = func(time);
	bool result = !j_pairs.first.empty();
	if (result) {
		vector<float>& joints = j_pairs.first;
		PlanningSystem::getInstance().BroadcastJointsAngle(obj->getName(), joints, time);
	}
#ifdef _DEBUG
	DebugPrintMessage(time, j_pairs);
#endif
	return result;
}

bool PlanningManager::TaskFinishedNotify(Model* key)
{
	task_time_stamp_ = -1.0f;
	auto iter = task_queue_map_.find(key);
	if (iter == task_queue_map_.end()) return false;	
	
	if (iter->second.empty()) {			// no task anymore, delete all resource from map 
		lock_map_.erase(key);
		execution_flag_map_.erase(key);
		task_queue_map_.erase(iter);
	}
	else {
		execution_flag_map_[key] = eExecution;
		QThreadPool::globalInstance()->start(iter->second.front());
		iter->second.pop();		
	}	
	return true;
}

}