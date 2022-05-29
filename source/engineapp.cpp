#include "engineapp.h"

#include "ui_mainwindow.h"

#include "manager/objectmanager.h"
#include "manager/modelmanager.h"
#include "manager/resourcemanager.h"

#include <iostream>
#include <format>

GComponent::EngineApp& GComponent::EngineApp::getInstance()
{
	static EngineApp instance;
	return instance;
}

template<class _TimeScale>
std::chrono::duration<float, _TimeScale> GComponent::EngineApp::GetSpanTime() {	
	steady_clock::time_point tick_now = steady_clock::now();
	duration<float, _TimeScale> time_span = duration_cast<microseconds>(tick_now - last_time_point_);
	last_time_point_ = tick_now;
	return time_span;
}

void GComponent::EngineApp::Init(int argc, char* argv[])
{
	gui_app_ptr_ = std::make_unique<QApplication>(argc, argv);

	// ui initialize
	window_ptr_ = std::make_unique<MainWindow>();
	window_ptr_->setWindowIconText("机械臂规划仿真软件");
	window_ptr_->setWindowFlag(Qt::FramelessWindowHint);

	GLModelTreeView* treeview	  = window_ptr_->getModelTreeView();
	UIState*		 ui_state_ptr = window_ptr_->getUIState();
	QComboBox*       obj_display  = window_ptr_->getModelDispaly();

	// model initialize
	GComponent::ObjectManager::getInstance().Initialize();
	model_tree_ = std::make_unique<EditorTreeModel>("");	
	
	// cross settings 
	treeview->setModel(model_tree_.get());
	
	// connections
	/* uimodel  >> treemodel */
	connect(treeview,						&GLModelTreeView::DeleteRequest,
			model_tree_.get(),				&EditorTreeModel::removeData);

	connect(model_tree_.get(),				&EditorTreeModel::DataDeletedNotice,
			treeview,						&GLModelTreeView::ResponseDataDeleted);
	connect(ui_state_ptr,					&UIState::DeleteRequest,
            model_tree_.get(),				&EditorTreeModel::ResponseDeleteRequest);
	
	 /* modelmanager >> treemodel */
    connect(&ModelManager::getInstance(),   &ModelManager::ModelRegisterNotice,
            model_tree_.get(),				&EditorTreeModel::ResponseCreateRequest);
    connect(&ModelManager::getInstance(),   &ModelManager::ModelParentChangeNotice,
            model_tree_.get(),				&EditorTreeModel::ResponseParentChangeRequest);

	 /* uimodel >> MODELMANAGER */
    connect(treeview,						 &GLModelTreeView::DeleteRequest,
            &ModelManager::getInstance(),    &ModelManager::ResponseDeleteRequest);                
    connect(ui_state_ptr,					 &UIState::DeleteRequest,
            &ModelManager::getInstance(),    &ModelManager::ResponseDeleteRequest);

	/* TREE_VIEW >> OBJECTMANAGER */
    connect(treeview,						 &GLModelTreeView::CreateRequest,
            &ObjectManager::getInstance(),   &ObjectManager::CreateInstance);

	/* ADPATER CONNECTIONS */
	/* UI_STATE  >> TREEVIEW */
	connect(ui_state_ptr,		&UIState::SelectRequest,
			[treeview = treeview, &tree = model_tree_](const string& name) {
				treeview->ResponseSelectRequest(tree->getIndexByName(name));
			});
	/* ComboBox >> TREEVIEW */
	connect(obj_display,		&QComboBox::textActivated,
			[treeview = treeview, &tree = model_tree_](const QString& name) {
				treeview->ResponseSelectRequest(tree->getIndexByName(name.toStdString()));
			});

	/* uimodel << MODELMANAGER */
	connect(&ModelManager::getInstance(), &ModelManager::ModelRegisterNotice,
			[obj_display = obj_display](const std::string& name, const std::string& _){
				obj_display->addItem(QString::fromStdString(name));
			});
	/* uimodel  << MODELMANAGER */
	connect(&ModelManager::getInstance(), &ModelManager::ModelDeregisterNotice,
			[obj_display = obj_display](const std::string& name){
				if (int index = obj_display->findText(QString::fromStdString(name));
					!name.empty() && index != -1) {
					obj_display->removeItem(index);
					obj_display->setCurrentIndex(0);
				}				
			});
}

int GComponent::EngineApp::Exec()
{
	window_ptr_->show();
	if (!gui_app_ptr_) return 0;
	return gui_app_ptr_->exec();
		
}

void GComponent::EngineApp::LogicTick(float delta_time_ms)
{

}

void GComponent::EngineApp::RenderTick(float delta_time_ms)
{
	//SceneManager::getInstance().tick(delta_time_ms);
}
