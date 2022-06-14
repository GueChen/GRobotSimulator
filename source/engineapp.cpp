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

	robot_create_dialog_ptr_ = std::make_unique<RobotCreateDialog>();	
	robot_create_dialog_ptr_->setWindowTitle("Create Ur Robot(*^_^*)");
	
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
	connect(treeview,						 &GLModelTreeView::RobotCreateRequest,
			robot_create_dialog_ptr_.get(),  &RobotCreateDialog::show);

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
	connect(robot_create_dialog_ptr_.get(), &RobotCreateDialog::RobotCreateRequestMDH,
			this,							&EngineApp::TestConversion);
	
	/* close all window */
	connect(window_ptr_.get(),				&MainWindow::deleteLater,
			robot_create_dialog_ptr_.get(), &RobotCreateDialog::close);
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
	
}

void GComponent::EngineApp::TestConversion(const vector<vector<float>>& params)
{
	LocalTransformsSE3<float> matrices = LocalTransformsSE3<float>::fromMDH(params);

	Model* base = new Model("robot_base", "", "color");
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
		
		std::cout << "the local trans: " << t.transpose() << std::endl;
		std::cout << "the local rot  : " << r.transpose() << std::endl;
		std::cout << transform_mat << std::endl;
		Eigen::Vector3f scale = 0.08f * Eigen::Vector3<float>::Ones();
		models[idx] = new Model(name + std::to_string(idx), 
								"sphere", 
								"color",
								t,
								r,
								scale,
								idx == 0 ? base: models[idx - 1]);										
		ModelManager::getInstance().RegisteredModel(models[idx]->getName(), models[idx]);
		
		// Create Joints Mesh
		scale = 0.04f * Eigen::Vector3<float>::Ones();
		scale.z() = 0.15f;
		Model* 
		joint_mesh_model = new Model("joint_mesh_" + std::to_string(idx),
									 "cylinder",
									 "color",
							 		 Eigen::Vector3<float>::Zero(),
									 Eigen::Vector3<float>::Zero(),
									 scale,
									 models[idx]);
		ModelManager::getInstance().RegisteredModel(joint_mesh_model->getName(), joint_mesh_model);

		// Create Link Mesh
		if (t.norm() > 1e-5) {
			scale = 0.03f * Eigen::Vector3<float>::Ones();
			scale.z() = t.norm();
			Vec3 r_link = GetRotateAxisAngleFrom2Vec(Vec3::UnitZ().eval(), t);
			Model* 
			link_mesh_model = new Model("link_mesh_" + std::to_string(idx),
										"cylinder",
										"color",
										t/2,
										r_link,
										scale,
										idx == 0 ? base: models[idx - 1]);
			ModelManager::getInstance().RegisteredModel(link_mesh_model->getName(), link_mesh_model);
		}
		
		// Register the Joint Component
		models[idx]->RegisterComponent(std::make_unique<JointComponent>(models[idx], (R * Eigen::Vector3f::UnitZ()).normalized()));
		joints.push_back(models[idx]->GetComponet<JointComponent>("JointComponent"));
		++idx;
	}
	auto [twists, T] = matrices.toTwists();
	base->RegisterComponent(make_unique<JointGroupComponent>(joints, base));
	base->RegisterComponent(make_unique<KinematicComponent>(T, base));
	//base->GetComponet<KinematicComponent>("KinematicComponent")->SetExpCoords(twists);
	std::cout << "The Init T:\n" << T << std::endl;
	std::cout << "twists val :\n";
	for (auto& twist : twists) {
		std::cout << "twist:" << twist.transpose() << std::endl;
	}

	base->RegisterComponent(make_unique<TrackerComponent>(base, "sphere0"));
}
