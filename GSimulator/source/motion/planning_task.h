/**
 *  @file  	planning_task.h
 *  @brief 	this class is a execution thread in QThread.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Sep 17th, 2022
 **/
#ifndef __PLANING_TASK_H
#define __PLANING_TASK_H

#include "motion/trajectory.h"

#include <QtCore/QRunnable>

#include <condition_variable>
#include <functional>
#include <mutex>

namespace GComponent {

class DefaultTaskPauseNotifyer {
public:
	void operator()(const std::string& task) const;
};

class DefaultTaskFinishNotifyer{
public:
	void operator()() const;
};

class DefaultCtrMessenger {
public:
	void operator()(const std::string& task, const JointPairs& input, float time) const;
};

class PlanningTask : public QRunnable{
/// Assistant Type
public:
	using _CtrMsgerFn    = std::function<void(const std::string& task, const JointPairs& input, float time)>;
	using _FinNotifyFn   = std::function<void()>;
	using _PauseNotifyFn = std::function<void(const std::string& task)>;
	template<class T>
	using _Ptr			 = std::shared_ptr<T>;
	using _PtrTrajs		 = std::vector<_Ptr<Trajectory>>;
	using _RefLocks		 = std::vector<std::mutex&>;

	enum TaskStatus :int {
		eStop		= 0, 
		eReadyPause = 1, 
		ePause		= 3, 
		eExecution  = 2,
		eBlocking	= 15,
		ePrepared   = 255, 
		eFinished   = (1 << 8)
	};

public:
// Constructor & Destructor
	PlanningTask(Trajectory* traj, std::mutex& lock);	

	~PlanningTask();

// The Task Interface
	virtual void	  run() override;
	virtual void	  operator()();

// Setter & Getter
	inline TaskStatus GetStatus()						   { return status_; }
	inline void		  SetStatus(TaskStatus status)		   { status_ = status; }

	inline void		  SetPauseNotifyer	 (_PauseNotifyFn fn){ pause_notifyer_	 = std::move(fn); }
	inline void		  SetFinishNotifyer  (_FinNotifyFn fn)  { finish_notifyer_   = std::move(fn); }
	inline void		  SetControlMessenger(_CtrMsgerFn fn)   { control_messenger_ = std::move(fn); }
	
	inline void		  NotifyInteruptOnce()				   { pause_notifyer_(func_->GetIdentifier()); }
protected:
	void			  ExecuteOnce(float time);
	void			  BeforeFinish();

/// Fields
protected:
	// original properties	
	_Ptr<Trajectory>		 func_;	

	// self status control
	TaskStatus				 status_		    = ePrepared;
	float					 caculate_time_	    = -1.0f;

	// assistants
	_FinNotifyFn			 finish_notifyer_	= DefaultTaskFinishNotifyer{};
	_CtrMsgerFn			     control_messenger_ = DefaultCtrMessenger{};
	_PauseNotifyFn		     pause_notifyer_	= DefaultTaskPauseNotifyer{};
	std::mutex&			     mt_;	
};

}

#endif // !__PLANING_TASK_H
