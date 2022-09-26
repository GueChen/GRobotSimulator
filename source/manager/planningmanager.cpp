#include "manager/planningmanager.h"

#include "system/planningsystem.h"

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

void PlanningManager::RegisterPlanningTask(Trajectory* traj) {
	Model*		  key	= &traj->GetModel();
	std::mutex&   lock	= lock_map_[key];
	auto&		  queue	= task_dict_[key];
	PlanningTask* task	= new PlanningTask(traj, lock);	
	task->SetFinishNotifyer([&queue = task_dict_[key]]() {
		auto task = queue.front(); 
		queue.pop_front();
		task->SetStatus(PlanningTask::eFinished);
		if (!queue.empty()) {
			QThreadPool::globalInstance()->start(queue.front());
		}	
		delete task;
	});
	queue.push_back(task);
	if (lock.try_lock()) {		
		QThreadPool::globalInstance()->start(task);
		lock.unlock();
	}	
}

void PlanningManager::RegisterDualPlanningTask(std::vector<Model*> robots, std::vector<JTrajFunc> func, float time_total, uint32_t interval_ms)
{
	auto task = [this, funcs = func, time_total, interval = interval_ms, 
				 m_keys = robots, 			
				 &m_locks = lock_map_, 
				 &m_que = task_queue_map_] {
		using namespace std::chrono;
		{
		std::lock_guard<std::mutex> lock_l(m_locks[m_keys[0]]),
									lock_r(m_locks[m_keys[1]]);

		float	    elapsed	   = 0.0f,
				    delay	   = 0.0f;
		
		time_point  start_time = steady_clock::now(),
					last_time  = steady_clock::now();			
		
		while (elapsed < time_total) {
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
	auto task_queue = task_dict_[robot];
	if (!task_queue.empty()) {
		task_queue.front()->SetStatus(static_cast<PlanningTask::TaskStatus>(status));
	}
}

PlanningTask::TaskStatus PlanningManager::GetCurrentTaskStatus(Model* robot)
{
	auto task_queue = task_dict_[robot];
	if (task_queue.empty()) return PlanningTask::TaskStatus();
	return task_queue.front()->GetStatus();
}

void PlanningManager::InteruptPrePlanning(Model* robot)
{
	auto task_queue = task_dict_[robot];
	if (!task_queue.empty()) {
		task_queue.front()->NotifyInteruptOnce();
	}
}

void PlanningManager::tick(float delta_time)
{}

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
		task_queue_map_.erase(iter);
	}
	else {		
		QThreadPool::globalInstance()->start(iter->second.front());
		iter->second.pop();		
	}	
	return true;
}

}