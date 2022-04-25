/**
 *  @file	Engine.h
 *  @brief	The Main Loop Control Program
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date	Apri 24, 2022
 **/
#ifndef _MYENGINE_H
#define _MYENGINE_H

#include "base/singleton.h"

#include <QtCore/QRunnable>
#include <QtCore/QObject>

#include <chrono>

namespace GComponent {
	using namespace std::chrono;
class Engine : public QRunnable, public SingletonBase<Engine>
{
	friend class SingletonBase<Engine>;
	NonCoyable(Engine)
private:
	bool					 need_quit_		  = false;
	steady_clock::time_point last_time_point_ = steady_clock::now();

public:
	// Í¨¹ý QRunnable ¼Ì³Ð
	virtual void run() override;

protected:
	Engine() = default;
	template<class _TimeScale>
	duration<float, _TimeScale> GetSpanTime();
	void LogicTick(float delta_time);
	void RenderTick();
};

}

#endif // !MyEngine




