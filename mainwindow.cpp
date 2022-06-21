#include "mainwindow.h"

#include "ui_mainwindow.h"

#include "manager/modelmanager.h"
#include "manager/editor/uistatemanager.h"
#include "function/adapter/component_ui_factory.h"

#include <QtWidgets/QCombobox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    updated_timer_ptr = new QTimer;
    this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::West);
    ui->logger->setContextMenuPolicy(Qt::CustomContextMenu);
    ConnectionInit();
   
}

MainWindow::~MainWindow()
{
    delete ui;
}

GComponent::UIState* MainWindow::getUIState() const
{
    return &ui->m_viewport->ui_state_;
}

GComponent::GLModelTreeView* MainWindow::getModelTreeView() const
{
    return ui->treeView;
}

QComboBox* MainWindow::getModelDispaly() const
{
    return ui->selected_combo;
}

void MainWindow::ConnectionInit()
{
    using namespace GComponent;
    connect(this, &QMainWindow::tabifiedDockWidgetActivated, this, &MainWindow::SetTabifyDockerWidgetQSS);
    connect(ui->m_viewport, &Viewport::EmitDeltaTime, this, &MainWindow::ReceiveDeltaTime);           
    /* UI_STATE << TREEVIEW */
    connect(ui->treeView,               &GComponent::GLModelTreeView::SelectRequest,
            &ui->m_viewport->ui_state_, &GComponent::UIState::ResponseSelectRequest); 
    connect(ui->treeView,               &GComponent::GLModelTreeView::SelectRequest,
            this,                       &MainWindow::CheckSelected);

    /* UI_STATE >> QComboBox */
    connect(&ui->m_viewport->ui_state_, &UIState::SelectRequest,
		    [obj_display = ui->selected_combo](const std::string& name) {		
			    if (int index = obj_display->findText(QString::fromStdString(name));
				    index != -1) {
				    obj_display->setCurrentIndex(index);
			    }
            });	   
    connect(&ui->m_viewport->ui_state_, &UIState::SelectRequest,
            this,                       &MainWindow::CheckSelected);

    /* UI_STATE << QComboBox */
    connect(ui->selected_combo,         &QComboBox::textActivated,
            [&ui_state = ui->m_viewport->ui_state_](const QString& name){
                ui_state.ResponseSelectRequest(name.toStdString());
            });

    /* QComboBox <<  TREEVIEW */    
    connect(ui->treeView,               &GComponent::GLModelTreeView::SelectRequest,
            [obj_display = ui->selected_combo](const std::string& name) {
			    if (int index = obj_display->findText(QString::fromStdString(name));
				    index != -1) {
				    obj_display->setCurrentIndex(index);
			    }
            });

    connect(updated_timer_ptr, &QTimer::timeout, this, &MainWindow::CheckSelected);
    updated_timer_ptr->start(100);

    
}



void MainWindow::ReceiveDeltaTime(float delta_time)
{
    ui->spanTimeData->setText(QString::number(delta_time, 10, 4));
    ui->FPSData->setText(QString::number(1.f / delta_time, 10, 2));
}

void MainWindow::SetTabifyDockerWidgetQSS(QDockWidget* widget)
{
    QTabBar * tabar = widget->parent()->findChild<QTabBar*>();
    if (tabar) {
        tabar->setStyleSheet(
            "QTabBar::tab{"
            "   min-width: 35px;"
            "}"
            "QTabBar::tab::selected{"
            "   background: rgb(50, 50, 50);"
            "   color: qlineargradient(spread:pad, x0: 0, y0: 0, x1: 1, y1: 1, stop: 0 rgb(125, 230, 175), stop: 1 rgb(75, 150, 150));"
            "}"
            "QTabBar::tab::!selected{"
            "   background: rgb(40, 40, 40);"
            "}"
        );
    }   
}

// TODO: remove Model from ui but supply a interface for setting
void MainWindow::CheckSelected()
{
    using namespace GComponent;
    static Model* last_ptr = nullptr;
    Model* selected_obj_ptr = ui->m_viewport->ui_state_.GetSelectedObject();
    
    if (last_ptr != selected_obj_ptr) {
        while (ui->componentstoolbox->count() > 2) {
            QWidget* w = ui->componentstoolbox->widget(2);
            ui->componentstoolbox->removeItem(2);
            delete w;
        }
    }

    if (!selected_obj_ptr) {
        if (last_ptr != selected_obj_ptr) {            
            for (QString text = "NULL"; auto& edit : ui->component_observer->findChildren<QLineEdit*>()) {
                edit->setText(text);
            }
        }
    }
    else {
        // Setting Properties
        Model* parent_ptr = selected_obj_ptr->getParent();
        ui->name_edit->setText(QString::fromStdString(selected_obj_ptr->getName()));
        ui->mesh_edit->setText(QString::fromStdString(selected_obj_ptr->getMesh()));
        ui->shader_edit->setText(QString::fromStdString(selected_obj_ptr->getShader()));
        ui->parent_edit->setText(QString::fromStdString(parent_ptr ? parent_ptr->getName() : "None"));

        // Setting Transforms
        Vec3 trans = selected_obj_ptr->getTransGlobal();        
        ui->trans_x_edit->setText(QString::number(trans.x(), 10, 2));
        ui->trans_y_edit->setText(QString::number(trans.y(), 10, 2));
        ui->trans_z_edit->setText(QString::number(trans.z(), 10, 2));
        Vec3 rot = selected_obj_ptr->getRotGlobal();
        ui->rot_x_edit->setText(QString::number(rot.x(), 10, 2));
        ui->rot_y_edit->setText(QString::number(rot.y(), 10, 2));
        ui->rot_z_edit->setText(QString::number(rot.z(), 10, 2));
        Vec3 scale = selected_obj_ptr->getScale();
        ui->scale_x_edit->setText(QString::number(scale.x(), 10, 2));
        ui->scale_y_edit->setText(QString::number(scale.y(), 10, 2));
        ui->scale_z_edit->setText(QString::number(scale.z(), 10, 2));

        if (last_ptr != selected_obj_ptr) {                
            for (auto& component : selected_obj_ptr->GetComponents()) {
                QString type_name(component->GetTypeName().data());
                type_name =type_name.mid(0, type_name.indexOf("Component"));
                ui->componentstoolbox->addItem(ComponentUIFactory::Create(*component), type_name);
            }
        }
    }
    last_ptr = selected_obj_ptr;
}

void MainWindow::on_check_button_clicked()
{
    ui->m_viewport->ui_state_.ResponseAxisModeChange(AxisMode::None);
}

void MainWindow::on_trans_button_clicked()
{
    ui->m_viewport->ui_state_.ResponseAxisModeChange(AxisMode::Translation);
}

void MainWindow::on_rot_button_clicked()
{
    ui->m_viewport->ui_state_.ResponseAxisModeChange(AxisMode::Rotation);
}

void MainWindow::on_scale_button_clicked()
{
    ui->m_viewport->ui_state_.ResponseAxisModeChange(AxisMode::Scale);
}

