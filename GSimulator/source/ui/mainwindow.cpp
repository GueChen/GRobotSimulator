#include "mainwindow.h"

#include "ui_mainwindow.h"

#include "base/global/global_qss.h"
#include "ui/menu/componentmenu.h"
#include "manager/modelmanager.h"
#include "manager/editor/uistatemanager.h"
#include "function/adapter/component_ui_factory.h"

#include "CollapsibleHeader/collapsible_header.h"

#include <QtWidgets/QCombobox>
#include <QtWidgets/QfileDialog>
#include <QtWidgets/QScrollArea>

#ifdef _DEBUG
#include <iostream>
#endif

static QVBoxLayout* component_layout = nullptr;

static void InsertComponetObserver(const QString& type_name, QWidget* observer_ctx) {
    CollapsibleHeader* observer_widget = new CollapsibleHeader(type_name, observer_ctx);

    int32_t  insert_pos = component_layout->count() - 1; // require igonre the spacer
    component_layout->insertWidget(insert_pos, observer_widget);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
    , component_menu_(new GComponent::ComponentMenu(this))
{
    ui_->setupUi(this);    
    updated_timer_ptr_ = new QTimer;
    this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::West);
    ui_->logger->setContextMenuPolicy(Qt::CustomContextMenu);        
    component_menu_->setStyleSheet(menu_qss.data());
    
    // Replace component toolbox
    //ui_->component_view->setContextMenuPolicy(Qt::CustomContextMenu);
    //ui_->component_view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);    
    //ui_->component_view->
    component_layout = ui_->component_view_layout;    

    //component_layout = new QVBoxLayout;
    //QScrollArea* sc_area = new QScrollArea;
    //sc_area->setLayout(component_layout);
    //sc_area->setWidgetResizable(true);

    //ui_->component_view_layout->insertWidget(0, sc_area);
        
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
    /*connect(component_menu_->m_remove_component_action, &QAction::triggered, [this]() {        
        if (int idx = ui_->componentstoolbox->currentIndex(); idx > 1)
        {
            emit RequestDeleteComponent(ui_->componentstoolbox->itemText(idx));
            ui_->componentstoolbox->setCurrentIndex(idx - 1);
            ui_->componentstoolbox->removeItem(idx);                        
        }}
    );*/

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
    if (!component_ptr) return;    
    
    QWidget* observer_ctx = GComponent::ComponentUIFactory::Create(*component_ptr);    
    InsertComponetObserver(type_name, observer_ctx);    
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
    
    
    if (last_ptr == selected_obj_ptr) return;
    
    // change tool box ctx
    // remove all observing component widgets
    uint32_t component_count = component_layout->count();
    while(component_count) {            
        QLayoutItem* item = component_layout->itemAt(component_count - 1);
        component_layout->removeItem(item);

        if (QWidget* w = item->widget()) {
            w->deleteLater();
        }

        --component_count;
    }

    // refill component widgets
    if (selected_obj_ptr) {
        for (auto& component : selected_obj_ptr->GetComponents()) {
            QString type_name = QString(component->GetTypeName().data());
            type_name = type_name.mid(0, type_name.indexOf("Component"));
            
            QWidget* observer_ctx = GComponent::ComponentUIFactory::Create(*component);
            InsertComponetObserver(type_name, observer_ctx);
        }
        component_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding));
    }
    
    // change observer item   
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

void MainWindow::on_save_action_triggered()
{
    GComponent::ModelManager::getInstance().Save();
}

void MainWindow::on_load_action_triggered()
{
    QString file_path = QFileDialog::getOpenFileName(this, "Select which Load", "./", "Json files(*.json)");
    QFile file(file_path);
    if (file_path.isEmpty() || !file.open(QIODevice::ReadOnly)) return;
    
    QByteArray json_data = file.readAll();
    file.close();
    QJsonDocument json_doc = QJsonDocument::fromJson(json_data);

    if (json_doc.isNull()) return;

    GComponent::ModelManager::getInstance().Load(json_doc);
}


