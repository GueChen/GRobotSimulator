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
	namespace sc = std::chrono;

class Engine : public QRunnable, public SingletonBase<Engine>
{
	friend class SingletonBase<Engine>;
	Q_OBJECT
	NonCoyable(Engine)
private:
	bool need_quit_ = false;
	sc::steady_clock::time_point last_tick_time_point{ sc::steady_clock::now() };

public:
	// Í¨¹ý QRunnable ¼Ì³Ð
	virtual void run() override;

protected:
	Engine() = default;
signals:
	void OpenGLWidgetDrawNow();
};

}

#endif // !MyEngine




