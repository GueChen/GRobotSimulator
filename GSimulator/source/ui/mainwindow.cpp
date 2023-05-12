#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "base/global/global_qss.h"
#include "ui/menu/componentmenu.h"
#include "manager/modelmanager.h"
#include "manager/editor/uistatemanager.h"
#include "function/adapter/component_ui_factory.h"

#include <QtWidgets/QCombobox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
    , component_menu_(new GComponent::ComponentMenu(this))
{
    ui_->setupUi(this);    
    updated_timer_ptr_ = new QTimer;
    this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::West);
    ui_->logger->setContextMenuPolicy(Qt::CustomContextMenu);    
    ui_->componentstoolbox->setContextMenuPolicy(Qt::CustomContextMenu);
    component_menu_->setStyleSheet(menu_qss.data());
    ConnectionInit();   
    
}

MainWindow::~MainWindow()
{
    updated_timer_ptr_->stop();
    delete updated_timer_ptr_;
    delete ui_;
    delete component_menu_;
}

GComponent::UIState* MainWindow::getUIState() const
{
    return &ui_->m_viewport->ui_state_;
}

GComponent::GLModelTreeView* MainWindow::getModelTreeView() const
{
    return ui_->treeView;
}

QComboBox* MainWindow::getModelDispaly() const
{
    return ui_->selected_combo;
}

GComponent::ComponentMenu* MainWindow::getComponentMenu() const
{
    return component_menu_;
}

void MainWindow::ConnectionInit()
{
    using namespace GComponent;
    connect(this, &QMainWindow::tabifiedDockWidgetActivated, this, &MainWindow::SetTabifyDockerWidgetQSS);
    connect(ui_->m_viewport, &Viewport::EmitDeltaTime, this, &MainWindow::ReceiveDeltaTime);           
    /* UI_STATE << TREEVIEW */
    connect(ui_->treeView,               &GComponent::GLModelTreeView::SelectRequest,
            &ui_->m_viewport->ui_state_, &GComponent::UIState::ResponseSelectRequest); 
    connect(ui_->treeView,               &GComponent::GLModelTreeView::SelectRequest,
            this,                       &MainWindow::CheckSelected);

    /* UI_STATE >> QComboBox */
    connect(&ui_->m_viewport->ui_state_, &UIState::SelectRequest,
		    [obj_display = ui_->selected_combo](const std::string& name) {		
			    if (int index = obj_display->findText(QString::fromStdString(name));
				    index != -1) {
				    obj_display->setCurrentIndex(index);
			    }
            });	   
    connect(&ui_->m_viewport->ui_state_, &UIState::SelectRequest,
            this,                       &MainWindow::CheckSelected);

    /* UI_STATE << QComboBox */
    connect(ui_->selected_combo,         &QComboBox::textActivated,
            [&ui_state = ui_->m_viewport->ui_state_](const QString& name){
                ui_state.ResponseSelectRequest(name.toStdString());
            });

    /* QComboBox <<  TREEVIEW */    
    connect(ui_->treeView,               &GComponent::GLModelTreeView::SelectRequest,
            [obj_display = ui_->selected_combo](const std::string& name) {
			    if (int index = obj_display->findText(QString::fromStdString(name));
				    index != -1) {
				    obj_display->setCurrentIndex(index);
			    }
            });

    connect(updated_timer_ptr_, &QTimer::timeout, this, &MainWindow::CheckSelected);
    connect(&ModelManager::getInstance(),               &ModelManager::destroyed,
            updated_timer_ptr_,                         &QTimer::stop);
    updated_timer_ptr_->start(100);    

    /* component tool box */
    connect(component_menu_->m_remove_component_action, &QAction::triggered, [this]() {        
        if (int idx = ui_->componentstoolbox->currentIndex(); idx > 1)
        {
            emit RequestDeleteComponent(ui_->componentstoolbox->itemText(idx));
            ui_->componentstoolbox->setCurrentIndex(idx - 1);
            ui_->componentstoolbox->removeItem(idx);                        
        }}
    );

    // TODO: FIX hard-code by reflect? methods 
    connect(component_menu_->m_add_joint_component,       &QAction::triggered, [this](){
        emit RequestAddComponent("JointComponent");
    });
    connect(component_menu_->m_add_joint_group_component, &QAction::triggered, [this]() {
        emit RequestAddComponent("JointGroupComponent");
    });
    connect(component_menu_->m_add_kinematic_component,   &QAction::triggered, [this]() {
        emit RequestAddComponent("KinematicComponent");
    });
    connect(component_menu_->m_add_tracker_component,     &QAction::triggered, [this]() {
        emit RequestAddComponent("TrackerComponent");
    });

    // TODO: add the complete menu action
    connect(ui_->planning_module_action,                  &QAction::triggered, [this]() {
        emit RequestShowDialog("PlanningDialog");
    });

    connect(ui_->network_module_action,                   &QAction::triggered, [this]() {
        emit RequestShowDialog("NetworkDialog");
    });

    connect(ui_->skin_module_action,                      &QAction::triggered, [this]() {
        emit RequestShowDialog("SkinDialog");
    });

    connect(ui_->shader_add_action,                       &QAction::triggered, [this](){
        emit RequestShowDialog("ShaderCreatorDialog");
    });

    connect(ui_->cube_add_action,                         &QAction::triggered, [this]() {
        emit CreateRequest("cube");
    });
    
    connect(ui_->sphere_add_action,                       &QAction::triggered, [this]() {
        emit CreateRequest("sphere");
    });

    connect(ui_->cylinder_add_action,                     &QAction::triggered, [this]() {
        emit CreateRequest("cylinder");
    });

    connect(ui_->capsule_add_action,                      &QAction::triggered, [this]() {
        emit CreateRequest("capsule");
    });

    connect(ui_->plane_add_action,                        &QAction::triggered, [this]() {
        emit CreateRequest("plane");
    });
    
    connect(ui_->robot_add_action,                        &QAction::triggered, [this](){
        emit CreateRobotRequest();
    });

    connect(ui_->quit_action,                             &QAction::triggered,
            this,                                         &MainWindow::close);


}


/*___________________SLOTS METHODS_____________________________________________________________*/
/*___________________PUBLIC SLOTS METHODS______________________________________________________*/
void MainWindow::ResponseComponentCreateRequest(GComponent::Component* component_ptr, const QString& type_name)
{
    if (component_ptr) {
        ui_->componentstoolbox->addItem(GComponent::ComponentUIFactory::Create(*component_ptr), type_name);
    }
}

/*___________________PRAVITE SLOTS METHODS_____________________________________________________*/
void MainWindow::ReceiveDeltaTime(float delta_time)
{
    ui_->spanTimeData->setText(QString::number(delta_time, 10, 4));
    ui_->FPSData->setText(QString::number(1.f / delta_time, 10, 2));
}

void MainWindow::SetTabifyDockerWidgetQSS(QDockWidget* widget)
{
    QTabBar * tabar = widget->parent()->findChild<QTabBar*>();
    if (tabar) {
        tabar->setStyleSheet(tabify_dockwidget_qss.data());
    }   
}

// TODO: remove Model from ui but supply a interface for setting
void MainWindow::CheckSelected()
{
    using namespace GComponent;
    static Model* last_ptr = nullptr;
    Model* selected_obj_ptr = ui_->m_viewport->ui_state_.GetSelectedObject();
    
    if (last_ptr != selected_obj_ptr) {
        uint32_t count = ui_->componentstoolbox->count();
        while (count > 1) {            
            QWidget* w = ui_->componentstoolbox->widget(count - 1);
            ui_->componentstoolbox->removeItem(count - 1);
            w->deleteLater();
            --count;
        }
    }

    if (!selected_obj_ptr) {
        if (last_ptr != selected_obj_ptr) {            
            for (QString text = "NULL"; auto & edit : ui_->componentstoolbox->widget(0)->findChildren<QLineEdit*>()) {
                edit->setText(text);
            }            
        }
    }
    else {
        // Setting Properties
        Model* parent_ptr = selected_obj_ptr->getParent();
        ui_->name_edit->setText(QString::fromStdString(selected_obj_ptr->getName()));
        ui_->mesh_edit->setText(QString::fromStdString(selected_obj_ptr->getMesh()));       
        ui_->parent_edit->setText(QString::fromStdString(parent_ptr ? parent_ptr->getName() : "None"));

        if (last_ptr != selected_obj_ptr) {                
            for (auto& component : selected_obj_ptr->GetComponents()) {
                QString type_name(component->GetTypeName().data());
                type_name =type_name.mid(0, type_name.indexOf("Component"));
                ui_->componentstoolbox->addItem(ComponentUIFactory::Create(*component), type_name);
            }
        }
    }
    last_ptr = selected_obj_ptr;
}

void MainWindow::on_check_button_clicked()
{
    ui_->m_viewport->ui_state_.ResponseAxisModeChange(AxisMode::None);
}

void MainWindow::on_trans_button_clicked()
{
    ui_->m_viewport->ui_state_.ResponseAxisModeChange(AxisMode::Translation);
}

void MainWindow::on_rot_button_clicked()
{
    ui_->m_viewport->ui_state_.ResponseAxisModeChange(AxisMode::Rotation);
}

void MainWindow::on_scale_button_clicked()
{
    ui_->m_viewport->ui_state_.ResponseAxisModeChange(AxisMode::Scale);
}

void MainWindow::on_componentstoolbox_customContextMenuRequested(const QPoint& pos)
{
    component_menu_->popup(ui_->componentstoolbox->mapToGlobal(pos));
}

