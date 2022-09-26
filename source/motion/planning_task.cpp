#include "motion/planning_task.h"

#include "system/planningsystem.h"

#include <chrono>

namespace GComponent {
/*_____________________________Default Task Finish Notifyer______________________*/
void DefaultTaskFinishNotifyer::operator()() const {
// TODO: add impl
	
}

/*______________________________Default Control Messenger________________________*/
void DefaultCtrMessenger::operator()(const std::string& task, const JointPairs& input, float time) const {
// TODO: add impl
	PlanningSystem::getInstance().BroadcastJointsAngle(task, input.first, time);
}

/*______________________________Default Pause Notifyer___________________________*/
void DefaultTaskPauseNotifyer::operator()(const std::string& task) const {
	PlanningSystem::getInstance().BroadcastTaskPause(task);
}

/*_________________________________Planning Task_________________________________*/
PlanningTask::PlanningTask(Trajectory* traj, std::mutex& lock):
	func_(traj),
	mt_(lock)
{	
	setAutoDelete(false);
}	

PlanningTask::~PlanningTask()
{
	//finish_notifyer_(traj_func_.GetIdentifier());
}

void PlanningTask::run(){
	operator()();
}

void GComponent::PlanningTask::operator()()
{
	using namespace std::chrono;
	
	float time_total = func_->GetTimeTotal();		

	if (isinf(time_total) || isnan(time_total)) {						// time ineffective
		return;
	}

	std::lock_guard<std::mutex> guard(mt_);	
	float&	   elapsed		 = caculate_time_;							// traj input time		  
	TaskStatus last_status	 = status_;									// record last status for pausing case usage
	time_point last_time	 = steady_clock::now();						// record last time_point
	bool	   has_notified_ = false;									// control single shot pause action
	// change status make start
	status_ = eExecution;
	elapsed = 0.0f;

	while (elapsed < time_total && status_ != eStop) {
		time_point cur_time = steady_clock::now();
		if (status_ == eStop) break;
		if ((status_ == ePause || last_status == ePause) && !has_notified_) {						
			pause_notifyer_(func_->GetIdentifier());
			has_notified_ = true;
		}
		else if (status_ == eExecution || status_ == eBlocking) {
			if (status_ == eExecution) {
				elapsed += duration_cast<duration<float>>(cur_time - last_time).count();
			}
			ExecuteOnce(elapsed);
			has_notified_ = false;
		}
		last_time   = cur_time;
		last_status = status_;

		std::this_thread::sleep_for(2ms);
	}

	BeforeFinish();	
	finish_notifyer_();
}

void PlanningTask::ExecuteOnce(float time)
{
	auto result = func_->operator()(time);
	if (!result.first.empty()) {
		control_messenger_(func_->GetIdentifier(), result, time);
	}
}

void PlanningTask::BeforeFinish()
{
	if (status_ == eExecution) {
		if(func_->GetType() == eCSpace){
			CTrajectory& traj = *dynamic_cast<CTrajectory*>(func_.get());
			while (traj.GetModifyVector().norm() > 1e-3f) {
				ExecuteOnce(traj.GetTimeTotal());
			}
		}

	}
	else if (status_ == eStop) {
		pause_notifyer_(func_->GetIdentifier());
	}	
}

}