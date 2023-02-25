/**
 *  @file  	planningsystem.h
 *  @brief 	This class is a bridge to link planning inteface and ui.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 22th, 2022
 **/
#ifndef __PLANNING_SYSTEM_H
#define __PLANNING_SYSTEM_H

#include "base/singleton.h"
#include "function/conversion.hpp"
#include "motion/optimization/skinsensor_optimizer.h"
#include "motion/optimization/trajectory_optimization.h"

#include <QtCore/QObject>

#include <vector>
#include <string>

namespace GComponent {
enum TrajDisplayFlag {
	InitPoint  = (1 << 0),
	GoalPoint  = (1 << 1),
	WayPoints  = (1 << 2),
	TrajSample = (1 << 3),
	TrajPath   = (1 << 30),

	AllConfig  = (TrajPath | InitPoint | GoalPoint)
};

class PlanningSystem : public QObject{
	Q_OBJECT

	NonCopyable(PlanningSystem)

public:
	static PlanningSystem& getInstance();
	~PlanningSystem();
	
	inline int  GetDisplayFlags() const	   { return display_flag; }
	inline void SetDisplayFlags(int flags) { display_flag = flags; }
	void		BroadcastJointsAngle
								(const std::string& name, std::vector<float> joints,
								 float				time_stamp);
	void		BroadcastTaskPause
								(const std::string& name);
protected:
	PlanningSystem() = default;

/// TEST_USE
	std::string SimpleGetObjName(const QString& obj_name);

public slots:
	void ResponsePTPMotionJSpace(const QString& obj_name, 
								 float max_vel,		float max_acc, 
								 std::vector<float> target_joints);
	void ResponsePTPMotionCSpace(const QString& obj_name,
								 float max_vel,		float max_acc,
								 std::vector<float> target_descarte);
	void ResponseLineMotion		(const QString& obj_name,
								 float max_vel,		float max_acc,
								 float max_ang_vel,	float max_ang_acc,
								 std::vector<float> target_descarte);
	void ResponseCircleMotion   (const QString& obj_name, 
								 float max_vel,		float max_acc, 
								 float max_ang_vel,	float max_ang_acc,
								 std::vector<float> target_descarte,
								 std::vector<float> waypoint);
	void ResponseSplineMotion   (const QString& obj_name,
							     float max_vel,		float max_acc,
								 float max_ang_vel,	float max_ang_acc,
							     std::vector<float> target_descarte,
							     std::vector<std::vector<float>> waypoints);
	void ResponseKeeperMotion   (const QString& obj_name,
								 float time_total, 
								 std::vector<float> target_descarte);
	void ResponseDualSyncLineMotion
							    (const std::vector<QString>& obj_names, 
							 	 std::vector<float> max_vels,  std::vector<float> max_accs, 
							 	 std::vector<float> target,   
							 	 std::vector<std::vector<float>> bias);
	void ResponseDualSyncCircleMotion
							     (const std::vector<QString>& obj_names, 
								  std::vector<float> max_vels,  std::vector<float> max_accs, 
								  std::vector<float> target,    
								  std::vector<std::vector<float>> bias,
								  std::vector<float> waypoint);	
	void ResponseChangeCurrentTaskStatus
							   (const std::vector<QString>& obj_name, int status);
	void SetTargetOptimizer(int idx);
	void SetSelfMotionOptimier(int idx);

signals:
	void		NotifyNewJointsAngle
							   (const QString& obj_name, std::vector<float> joints,
							    float		   time_stamp);
	void		NotifyPauseTask(const QString& obj_name);

private:
	TgtOptimizer* GetTgtOptimizer();
	SlfOptimizer* GetSlfOptimizer();

private:
	int display_flag = 0;
	int tgt_flag = 0;
	int slf_flag = 0;

};

} // !namespace GComponent

#endif // !__PLANNING_SYSTEM_H
