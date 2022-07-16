/**
 *  @file	Engine.h
 *  @brief	The Main Loop Control Program
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date	Apri 24, 2022
 **/
#ifndef _MYENGINE_H
#define _MYENGINE_H

#include "ui/dialog/robotcreatedialog.h"

#include "base/singleton.h"
#include "base/editortreemodel.h"
#include "mainwindow.h"

#include <QtCore/QObject>
#include <QtWidgets/QApplication>

#include <memory>
#include <chrono>

#include "component/joint_component.h"
#include "component/joint_group_component.h"
#include "component/kinematic_component.h"
#include "component/tracker_component.h"
#include "function/adapter/kinematic_adapter.h"
#include <GComponent/GGeometry.hpp>
#ifdef _DEBUG
 /*******************/ //DELETE THIS AFTER TEST

/******************/
#endif // !_DEBUG

namespace GComponent {
using namespace std::chrono;
using std::unique_ptr;

class Component;

class EngineApp : public QObject
{
	Q_OBJECT
	NonCoyable(EngineApp)
public:
	static EngineApp& getInstance(); 
	virtual ~EngineApp() = default;
	
	void Init(int argc, char* argv[]);
	int Exec();	

protected:
	EngineApp() = default;
	template<class _TimeScale>
	duration<float, _TimeScale> GetSpanTime();	

// FIXME: puts it into a proper position
	void TestConversion(const vector<vector<float>>& params);

signals:
	void RequestCreateComponentUI(Component* component_ptr, const QString& com_name);

private:
	unique_ptr<QApplication>		gui_app_ptr_			 = nullptr;
	unique_ptr<EditorTreeModel>		model_tree_				 = nullptr;
	unique_ptr<MainWindow>			window_ptr_				 = nullptr;
	unique_ptr<RobotCreateDialog>	robot_create_dialog_ptr_ = nullptr;
	steady_clock::time_point		last_time_point_		 = steady_clock::now();
};

}

#endif // !MyEngine




