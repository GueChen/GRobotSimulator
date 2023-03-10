/**
 *  @file  	planningmanager.h
 *  @brief 	This manager responsible for the model trajectory planning.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	8 20th, 2022
 **/
#ifndef __PLANNING_MANAGER_H
#define __PLANNING_MANAGER_H

#include "base/singleton.h"

#include "motion/gmotionbase.h"
#include "motion/planning_task.h"

#include <unordered_map>
#include <functional>
#include <thread>
#include <deque>

namespace GComponent {

class Model;

class PlanningManager{	
	enum TaskStatus:int {
		eStop = 0, eReadyPause = 1, eExecution = 2, ePause = 10
	};

	NonCopyable(PlanningManager)

public:
	static PlanningManager& getInstance();
	~PlanningManager();
	
	void RegisterDualPlanningTask(std::vector<Model*> robots, std::vector<JTrajFunc> funcs, float time_total, uint32_t interval_ms);
	
	void RegisterPlanningTask(Trajectory* traj);

	void ChangeCurrentTaskStatus(Model* robot, int status);
	PlanningTask::TaskStatus
		 GetCurrentTaskStatus(Model* robot);
	void InteruptPrePlanning(Model* robot);

	inline float GetExecutionTime() const { return execution_time_stamp_; }
	inline float GetPlanningTime()  const { return task_time_stamp_; }
	
	void tick(float delta_time);

protected:
	PlanningManager() = default;

	bool TaskFinishedNotify(Model* key);
	bool ExecutionOnce(const JTrajFunc& func, Model* obj, float time) const;
private:
	std::unordered_map<Model*, std::mutex>						  lock_map_;	
	std::unordered_map<Model*, std::queue<std::function<void()>>> task_queue_map_;
	std::unordered_map<Model*, std::deque<PlanningTask*>>		  task_dict_;	
	
	float task_time_stamp_		= -1.0f;
	float execution_time_stamp_ = -1.0f;
};

} // !namespace GComponent

#endif // !__PLANNING_MANAGER_H
