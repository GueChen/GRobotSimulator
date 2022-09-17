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

	NonCoyable(PlanningManager)

public:
	static PlanningManager& getInstance();
	~PlanningManager();

	void RegisterPlanningTask(Model * robot, JTrajFunc func, float time_total, uint32_t interval_ms);

	void RegisterDualPlanningTask(std::vector<Model*> robots, std::vector<JTrajFunc> funcs, float time_total, uint32_t interval_ms);
		
	void ChangeCurrentTaskStatus(Model* robot, int status);

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
	std::unordered_map<Model*, TaskStatus>						  execution_flag_map_;
	/*std::unordered_map<Model*, std::deque<std::variant<Eigen::VectorXf, SE3f>>>
																  goal_deque_map_;*/

	float task_time_stamp_		= -1.0f;
	float execution_time_stamp_ = -1.0f;
};

} // !namespace GComponent

#endif // !__PLANNING_MANAGER_H
