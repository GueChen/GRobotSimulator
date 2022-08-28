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
#include "motion/dualsynclinemotion.h"

#include <QtCore/QObject>

#include <unordered_map>
#include <functional>
#include <thread>

namespace GComponent {

class Model;

class PlanningManager{	

	NonCoyable(PlanningManager)

public:
	static PlanningManager& getInstance();
	~PlanningManager();

	void RegisterPlanningTask(Model * robot, JTrajFunc func, float time_total, uint32_t interval_ms);

	void RegisterDualPlanningTask(std::vector<Model*> robots, std::vector<JTrajFunc> funcs, float time_total, uint32_t interval_ms);
	
	void tick(float delta_time);
protected:
	PlanningManager() = default;

private:
	std::unordered_map<Model*, std::mutex>						  lock_map;
	std::unordered_map<Model*, std::queue<std::function<void()>>> task_queue_map;
	
};

} // !namespace GComponent

#endif // !__PLANNING_MANAGER_H
