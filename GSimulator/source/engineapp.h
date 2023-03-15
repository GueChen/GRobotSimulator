/**
 *  @file	Engine.h
 *  @brief	The Main Loop Control Program
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date	Apri 24, 2022
 **/
#ifndef _MYENGINE_H
#define _MYENGINE_H


#include "base/singleton.h"

#include <QtCore/QObject>
#include <QtWidgets/QApplication>

#include <memory>
#include <chrono>
#include <functional>

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

class MainWindow;
class QThread;

namespace GComponent{
// Function Related Class
class Component;
// UI Ralated Class
class EditorTreeModel;
class PlanningDialog;
class RobotCreatorDialog;
class NetworkDialog;
class SkinDialog;
class ShaderCreatorDialog;
}

namespace GComponent {

using namespace std::chrono;
using std::unique_ptr;

class EngineApp : public QObject
{
	template<class T>
	using _Deleter    = std::function<void(T*)>;
	template<class T>
	using _PtrWithDel = unique_ptr<T, _Deleter<T>>;

	Q_OBJECT
	NonCopyable(EngineApp)
public:
	static EngineApp& getInstance(); 
	virtual ~EngineApp();
	
	void Init(int argc, char* argv[]);
	int Exec();	

protected:
	EngineApp() = default;
	template<class _TimeScale>
	duration<float, _TimeScale> GetSpanTime();	

// FIXME: puts it into a proper position
	void CreateRobotWithParams(const vector<vector<float>>& params);
// FIXME: puts it into a proper position
	void CreateShader(const QString& name, const QString& vert, const QString& frag, const QString& geom);

private:
	void ConnectModules();
	void InitializeMembers(int argc, char* argv[]);
	void MoveSomeToThread();

signals:
	void RequestCreateComponentUI(Component* component_ptr, const QString& com_name);

private:
	unique_ptr<QApplication>		gui_app_ptr_				= nullptr;
	_PtrWithDel<EditorTreeModel>	model_tree_					= nullptr;
	_PtrWithDel<MainWindow>			window_ptr_					= nullptr;
	_PtrWithDel<RobotCreatorDialog>  robot_create_dialog_ptr_	= nullptr;
	_PtrWithDel<PlanningDialog>		planning_dialog_ptr_		= nullptr;
	_PtrWithDel<NetworkDialog>		network_dialog_ptr_			= nullptr;
	_PtrWithDel<SkinDialog>         skin_dialog_ptr_			= nullptr;
	_PtrWithDel<ShaderCreatorDialog>shader_creator_dialog_ptr_	= nullptr;
	//unique_ptr<>
	steady_clock::time_point		last_time_point_			= steady_clock::now();
	std::unordered_map<QString, QThread*> 
									threads_map_				= {};
};

}

#endif // !MyEngine




