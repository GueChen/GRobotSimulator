#include "engineapp.h"

#include "ui_mainwindow.h"

#include "ui/treeview/glmodeltreeview.h"
#include "ui/widget/viewport.h"

#include "ui/mainwindow.h"
#include "ui/dialog/Dialogs.h"
#include "base/editortreemodel.h"

#include "component/component_factory.h"
#include "manager/objectmanager.h"
#include "manager/modelmanager.h"
#include "manager/resourcemanager.h"
#include "manager/physicsmanager.h"
#include "manager/tcpsocketmanager.h"

#include "system/planningsystem.h"
#include "system/networksystem.h"
#include "system/transmitsystem.h"
#include "system/skinsystem.h"

#include "component/material_component.h"

#include <QtCore/QThread>
#include <QtWidgets/QMessageBox>
#include <unordered_map>

#ifndef _DEBUG
#include <iostream>
#include <format>
#endif // !_DEBUG

static std::unordered_map<QString, QDialog*> dialog_table;

GComponent::EngineApp& GComponent::EngineApp::getInstance()
{
	static EngineApp instance;
	return instance;
}

GComponent::EngineApp::~EngineApp()
{
	for (auto&& [__, thread] : threads_map_) {		
		thread->terminate();
		thread->wait();
	}
}

template<class _TimeScale>
std::chrono::duration<float, _TimeScale> GComponent::EngineApp::GetSpanTime() {	
	steady_clock::time_point	tick_now	= steady_clock::now();
	duration<float, _TimeScale> time_span	= duration_cast<microseconds>(tick_now - last_time_point_);
	last_time_point_ = tick_now;
	return time_span;
}

void GComponent::EngineApp::Init(int argc, char* argv[])
{
	// Manger Initializations
	PhysicsManager::getInstance().Initialize();
	ObjectManager::getInstance().Initialize();
	TcpSocketManager::getInstance().Initialize();
	NetworkSystem::getInstance().Initialize();
	SkinSystem::getInstance().Initialize();
	PhysicsManager::getInstance().SetActivateScene(PhysicsManager::getInstance().CreatePhysicsScene());

	InitializeMembers(argc, argv);
		
	// cross settings 
	window_ptr_->getModelTreeView()->setModel(model_tree_.get());
	
	ConnectModules();	
	for (int i = 0; i < (SkinSystem::getInstance()._Nlist).size(); i++)
	{
		skin_dialog_ptr_.get()->ComboText()->addItem((SkinSystem::getInstance()._Nlist)[i]);
	}

	MoveSomeToThread();
}

int GComponent::EngineApp::Exec()
{
	window_ptr_->show();
	if (!gui_app_ptr_) return 0;
	return gui_app_ptr_->exec();
		
}

// TODO: find a better place not in the engine, maybe object manager related
void GComponent::EngineApp::CreateRobotWithParams(const vector<vector<float>>& params)
{
	LocalTransformsSE3<float> matrices = LocalTransformsSE3<float>::fromMDH(params);

	Model* base = new Model("robot_base", "");
	base->RegisterComponent(std::make_unique<MaterialComponent>(base, "color", true));
	std::vector<JointComponent*> joints;
	ModelManager::getInstance().RegisteredModel(base->getName(), base);

	vector<Model*> models(matrices.size(), nullptr);
	string name = "robot_joint_";
	Eigen::Matrix4f transform_mat;
	transform_mat.setIdentity();
	for (int idx = 0; auto& mat : matrices.getMatrices()) 
	{
		// Get Neccessary Datas
		transform_mat = transform_mat * mat;
		auto [r, t] = rtDecompositionMat4(mat);
		auto [R, _] = RtDecompositionMat4(mat);
				
		Eigen::Vector3f scale = 0.08f * Eigen::Vector3<float>::Ones();
		models[idx] = new Model(name + std::to_string(idx), 
								"sphere", 								
								t,
								r,
								scale,
								idx == 0 ? base: models[idx - 1]);
		models[idx]->RegisterComponent(std::make_unique<MaterialComponent>(models[idx], "color", true));
		ModelManager::getInstance().RegisteredModel(models[idx]->getName(), models[idx]);
		
		// Create Joints Mesh
		scale = 0.04f * Eigen::Vector3<float>::Ones();
		scale.z() = 0.15f;
		Model* 
		joint_mesh_model = new Model("joint_mesh_" + std::to_string(idx),
									 "cylinder",
							 		 Eigen::Vector3<float>::Zero(),
									 Eigen::Vector3<float>::Zero(),
									 scale,
									 models[idx]);
		joint_mesh_model->RegisterComponent(std::make_unique<MaterialComponent>(joint_mesh_model, "color", true));
		ModelManager::getInstance().RegisteredModel(joint_mesh_model->getName(), joint_mesh_model);

		// Create Link Mesh
		if (t.norm() > 1e-5) {
			scale = 0.03f * Eigen::Vector3<float>::Ones();
			scale.z() = t.norm();
			Vec3 r_link = GetRotateAxisAngleFrom2Vec(Vec3::UnitZ().eval(), t);
			Model* 
			link_mesh_model = new Model("link_mesh_" + std::to_string(idx),
										"cylinder",										
										t/2,
										r_link,
										scale,
										idx == 0 ? base: models[idx - 1]);
			link_mesh_model->RegisterComponent(std::make_unique<MaterialComponent>(link_mesh_model, "color", true));
			ModelManager::getInstance().RegisteredModel(link_mesh_model->getName(), link_mesh_model);
		}
		
		// Register the Joint Component
		models[idx]->RegisterComponent(std::make_unique<JointComponent>(models[idx], (R * Eigen::Vector3f::UnitZ()).normalized()));
		joints.push_back(models[idx]->GetComponent<JointComponent>("JointComponent"));
		++idx;
	}
	auto [twists, T] = matrices.toTwists();

	base->RegisterComponent(make_unique<JointGroupComponent>(base, joints));
	base->RegisterComponent(make_unique<KinematicComponent>(T, base));		
	base->RegisterComponent(make_unique<TrackerComponent>(base, ""));
}

void GComponent::EngineApp::CreateShader(const QString& name, const QString& vert, const QString& frag, const QString& geom)
{
	if (ResourceManager::getInstance().GetShaderByName(name.toStdString())) {
		QMessageBox::warning(nullptr, "same name find", "already exist same name Shader", QMessageBox::Ok);
		return;
	}
	ResourceManager::getInstance().RegisteredShader(name.toStdString(), new MyShader(nullptr, vert.toStdString(), frag.toStdString(), geom.toStdString()));
}

/*_________________________________________PRIVATE METHODS______________________________________________________*/
void GComponent::EngineApp::ConnectModules()
{
	// connections
	GLModelTreeView* treeview	  = window_ptr_->getModelTreeView();
	UIState*		 ui_state_ptr = window_ptr_->getUIState();
	QComboBox*       obj_display  = window_ptr_->getModelDispaly();
	ComponentMenu*	 com_menu_ptr = window_ptr_->getComponentMenu();
	
	/* uimodel  >> treemodel */
	connect(treeview,						&GLModelTreeView::DeleteRequest,
			model_tree_.get(),				&EditorTreeModel::removeData);
	connect(model_tree_.get(),				&EditorTreeModel::ParentChangeRequest,
			&ModelManager::getInstance(),	&ModelManager::ResponseParentChangeRequest);
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
	connect(treeview,						 &GLModelTreeView::RobotCreateRequest,
			robot_create_dialog_ptr_.get(),  &RobotCreatorDialog::show);

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

	/* robotcreatedialog >> ? */
	connect(robot_create_dialog_ptr_.get(), &RobotCreatorDialog::RobotCreateRequestMDH,
			this,							&EngineApp::CreateRobotWithParams);
	
	/* close all window */
	/*connect(window_ptr_.get(),				&MainWindow::deleteLater,
			robot_create_dialog_ptr_.get(), &RobotCreateDialog::close);*/

	/* main_window >> planning_dialog */
	connect(window_ptr_.get(),				&MainWindow::RequestShowDialog,
			[this](const QString& dialog_name) 
		{
			dialog_table[dialog_name]->show();
		}
	);


	/* component tool box */
	connect(window_ptr_.get(),				&MainWindow::RequestDeleteComponent,
			[&model_manager = ModelManager::getInstance(), ui_state_ptr](const QString& com_name)
	{
		Model* model = ui_state_ptr->GetSelectedObject();
		if (model) model->DeregisterComponent(com_name.toStdString() + "Component");		
	});

	connect(window_ptr_.get(),				&MainWindow::RequestAddComponent,
		[window_ptr = window_ptr_.get(), ui_state_ptr](const QString& com_name) {
			Model* model = ui_state_ptr->GetSelectedObject();
			if (model) {
				if (model->RegisterComponent(ComponentFactory::GetComponent(com_name.toStdString(), model))) {
					Component* component = model->GetComponent<Component>(com_name.toStdString());
					window_ptr->ResponseComponentCreateRequest(component, com_name);
				}
			}
	});

	connect(window_ptr_.get(),				&MainWindow::CreateRequest,
			&ObjectManager::getInstance(),  &ObjectManager::CreateInstance);
	connect(window_ptr_.get(),				&MainWindow::CreateRobotRequest,
			robot_create_dialog_ptr_.get(), &RobotCreatorDialog::show);

	/* planning usage */
	// JPTP
	connect(planning_dialog_ptr_.get(),			&PlanningDialog::RequestPTPMotionJSpace,
			&PlanningSystem::getInstance(),		&PlanningSystem::ResponsePTPMotionJSpace);
	// CPTP
	connect(planning_dialog_ptr_.get(),			&PlanningDialog::RequestPTPMotionCSpace,
			&PlanningSystem::getInstance(),		&PlanningSystem::ResponsePTPMotionCSpace);
	// Line
	connect(planning_dialog_ptr_.get(),			&PlanningDialog::RequestLineMotion,
			&PlanningSystem::getInstance(),		&PlanningSystem::ResponseLineMotion);
	// Circle
	connect(planning_dialog_ptr_.get(),			&PlanningDialog::RequestCircleMotion,
			&PlanningSystem::getInstance(),		&PlanningSystem::ResponseCircleMotion);
	// Spline
	connect(planning_dialog_ptr_.get(),			&PlanningDialog::RequestSplineMotion,
			&PlanningSystem::getInstance(),		&PlanningSystem::ResponseSplineMotion);
	// Keeper
	connect(planning_dialog_ptr_.get(),			&PlanningDialog::RequestKeeperMotion,
			&PlanningSystem::getInstance(),		&PlanningSystem::ResponseKeeperMotion);
	// Line Sync
	connect(planning_dialog_ptr_.get(),			&PlanningDialog::RequestDualSyncLineMotion,
			&PlanningSystem::getInstance(),		&PlanningSystem::ResponseDualSyncLineMotion);
	// Circle Sync
	connect(planning_dialog_ptr_.get(),			&PlanningDialog::RequestDualSyncCircleMotion,
			&PlanningSystem::getInstance(),		&PlanningSystem::ResponseDualSyncCircleMotion);
	// Stop Motion
	connect(planning_dialog_ptr_.get(),			&PlanningDialog::RequestChangeCurrentTaskStatus,
			&PlanningSystem::getInstance(),		&PlanningSystem::ResponseChangeCurrentTaskStatus);

	connect(&PlanningSystem::getInstance(),		&PlanningSystem::NotifyNewJointsAngle,
			&TransmitSystem::getInstance(),		&TransmitSystem::ReceiveJointsAngle);

	connect(planning_dialog_ptr_.get(),			&PlanningDialog::GetTargetOptimizer,
			&PlanningSystem::getInstance(),		&PlanningSystem::SetTargetOptimizer);

	connect(planning_dialog_ptr_.get(),			&PlanningDialog::GetSelfMotionOptimizer,
			&PlanningSystem::getInstance(),		&PlanningSystem::SetSelfMotionOptimier);

	

	connect(&PlanningSystem::getInstance(),		&PlanningSystem::NotifyPauseTask,
			&TransmitSystem::getInstance(),		&TransmitSystem::ResponsePauseTask);

	/* network usage */
	connect(network_dialog_ptr_.get(),			&NetworkDialog::RequestLinkClientToServer,
			&NetworkSystem::getInstance(),		&NetworkSystem::ResponseLinkClientToServer);
	connect(network_dialog_ptr_.get(),			&NetworkDialog::RequestAsyncReceiver,
			&NetworkSystem::getInstance(),		&NetworkSystem::ResponseAsyncReceiver);
	connect(network_dialog_ptr_.get(),			&NetworkDialog::RequestQuit,
			&NetworkSystem::getInstance(),		&NetworkSystem::ResponseQuit);
	connect(network_dialog_ptr_.get(),			&NetworkDialog::RequestHigherAurthority,
			&NetworkSystem::getInstance(),		&NetworkSystem::ResponseHigherAurthority);
	connect(network_dialog_ptr_.get(),			&NetworkDialog::RequestModeChange,
			&TransmitSystem::getInstance(),		&TransmitSystem::ResponseModeChange);

	connect(&TcpSocketManager::getInstance(),   &TcpSocketManager::NotifySocketLinkState,
			network_dialog_ptr_.get(),			&NetworkDialog::LinkStateChange);
	connect(&TcpSocketManager::getInstance(),   &TcpSocketManager::NotifySocketRank,
			network_dialog_ptr_.get(),			&NetworkDialog::RankLevelChange);
	connect(&TcpSocketManager::getInstance(),   &TcpSocketManager::NotifyAsyncStatus,
			network_dialog_ptr_.get(),			&NetworkDialog::ResponseAsyncStatus);
	connect(&TcpSocketManager::getInstance(),	&TcpSocketManager::TransmitRobotDatas,
			&TransmitSystem::getInstance(),		&TransmitSystem::ProcessRobotTransmitDatas);

	connect(&PlanningSystem::getInstance(),		&PlanningSystem::NotifyNewJointsAngle,
			&TransmitSystem::getInstance(),		&TransmitSystem::ReceiveJointsAngle);	

	connect(&TransmitSystem::getInstance(),		&TransmitSystem::SendPlanningDatas,
			&NetworkSystem::getInstance(),		&NetworkSystem::ResponseSendJointsAngle);
	connect(&TransmitSystem::getInstance(),		&TransmitSystem::SendCancelRequest,
			&NetworkSystem::getInstance(),		&NetworkSystem::ResponseSendCancelRequest);
	connect(&NetworkSystem::getInstance(),		&NetworkSystem::NotifySocketLinkError,
			network_dialog_ptr_.get(),			&NetworkDialog::ResponseLinkError);


	/* skin usage */
	connect(skin_dialog_ptr_.get(),				&SkinDialog::RunCtr, 
			&SkinSystem::getInstance(),			&SkinSystem::Run);
	connect(skin_dialog_ptr_.get(),				&SkinDialog::CloseCtr, 
			&SkinSystem::getInstance(),			&SkinSystem::Close);
	connect(skin_dialog_ptr_.get(),				&SkinDialog::GetPortName, 
			&SkinSystem::getInstance(),			&SkinSystem::GetPort);
	connect(&SkinSystem::getInstance(),			&SkinSystem::SendValue,
			skin_dialog_ptr_.get(),				&SkinDialog::GetValue, Qt::DirectConnection);//DirectConnection
	/*connect(&SkinSystem::getInstance(),			&SkinSystem::SendString,
			skin_dialog_ptr_.get(),				&SkinDialog::DisplayValue, Qt::DirectConnection);*/
	connect(skin_dialog_ptr_.get(),				&SkinDialog::SetZero, 
			&SkinSystem::getInstance(),			&SkinSystem::GetBase);
	connect(skin_dialog_ptr_.get(),				&SkinDialog::SetTrue, 
			&SkinSystem::getInstance(),			&SkinSystem::ClearBase);
	connect(skin_dialog_ptr_.get(),				&SkinDialog::ReadPort,
			&SkinSystem::getInstance(),			&SkinSystem::ReadPortName);
	connect(&SkinSystem::getInstance(),			&SkinSystem::SendPortList,
			skin_dialog_ptr_.get(),				&SkinDialog::GetPortlist, Qt::DirectConnection);
	connect(skin_dialog_ptr_.get(),				&SkinDialog::SetDirectionVector,
			&SkinSystem::getInstance(),			&SkinSystem::SetSensorVector);
	connect(skin_dialog_ptr_.get(),				&SkinDialog::SetSkinUsedMask,
			&SkinSystem::getInstance(),			&SkinSystem::SetUsingMask);

	connect(shader_creator_dialog_ptr_.get(), &ShaderCreatorDialog::ShaderCreateRequest,
			this,							  &EngineApp::CreateShader);

	connect(&ResourceManager::getInstance(), &ResourceManager::ShaderRegistered,
		[dialog = shader_creator_dialog_ptr_.get()](const std::string& name){
		dialog->AddShader(QString::fromStdString(name));
	});
}



void GComponent::EngineApp::InitializeMembers(int argc, char* argv[])
{
	auto deleter = [](auto ptr) {delete ptr; };
	// QApplication initialize
	gui_app_ptr_ = std::make_unique<QApplication>(argc, argv);

	// ui initialize
	window_ptr_ = _PtrWithDel<MainWindow>(new MainWindow, deleter);
	window_ptr_->setWindowIconText("机械臂规划仿真软件");
	//window_ptr_->setWindowFlag(Qt::FramelessWindowHint);

	// create dialog ptr and register them to table
#define PTR_CREATE(ptr_name, Type) \
do{	\
	ptr_name = _PtrWithDel<Type>(new Type(window_ptr_.get()), deleter); \
	dialog_table.emplace(QString((#Type)), (ptr_name).get());\
}while(false)

	PTR_CREATE(robot_create_dialog_ptr_,	RobotCreatorDialog);	
	PTR_CREATE(planning_dialog_ptr_,		PlanningDialog);	
	PTR_CREATE(network_dialog_ptr_,			NetworkDialog);	
	PTR_CREATE(skin_dialog_ptr_,			SkinDialog);	
	PTR_CREATE(shader_creator_dialog_ptr_,	ShaderCreatorDialog);
	
#undef PTR_CREATE
	
	robot_create_dialog_ptr_->setWindowTitle("Create Ur Robot(*^_^*)");
	planning_dialog_ptr_	->setWindowTitle("Planning As ur Wish");
	network_dialog_ptr_		->setWindowTitle("Link the World");
	skin_dialog_ptr_		->setWindowTitle("Read sensor data");

	// model initialize	
	model_tree_ = _PtrWithDel<EditorTreeModel>(new EditorTreeModel(""), deleter);
}



void GComponent::EngineApp::MoveSomeToThread()
{
	threads_map_["Planning"]	= new QThread;
	PlanningSystem::getInstance().moveToThread(threads_map_["Planning"]);

	threads_map_["Network"]		= new QThread;
	NetworkSystem::getInstance().moveToThread(threads_map_["Network"]);
	
	threads_map_["TcpSocket"]	= new QThread;
	TcpSocketManager::getInstance().moveToThread(threads_map_["TcpSocket"]);

	threads_map_["Skin"]		= new QThread;
	SkinSystem::getInstance().moveToThread(threads_map_["Skin"]);

	for (auto&& [__, t_thread] : threads_map_) {
		t_thread->start();
	}
}

