#include "mainwindow.h"

#include "ui_mainwindow.h"

#include "manager/modelmanager.h"
#include "function/adapter/component_ui_factory.h"

#include <QtWidgets/QCombobox>

#define __TEST__PLANNING
#ifdef __TEST__PLANNING
/// Just for Test Include

#include "motion/GMotion"
#include "model/robot/kuka_iiwa_model.h"
#include "component/joint_group_component.h"

#include "function/conversion.hpp"

#include <QtWidgets/QGridLayout>
#include <QWidget>
#include <QtWidgets/QTabBar>
#include <QtCharts/QLegend>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <eigen3/Eigen/Eigenvalues>

typedef QPair<QPointF, QString> Data;
typedef QList<Data> DataList;
typedef QList<DataList> DataTable;
using namespace GComponent;
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    updated_timer_ptr = new QTimer;
    this->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::West);
    ui->logger->setContextMenuPolicy(Qt::CustomContextMenu);
    ConnectionInit();
   
//#ifdef __TEST__PLANNING
//    QChart * chart = new QChart();
//    chart->createDefaultAxes();
//    chart->setTheme(QChart::ChartThemeDark);
//    chart->legend()->setAlignment(Qt::AlignRight);
//    m_chart = new QChartView(chart);
//    ui->PlanningLayout->addWidget(m_chart, 4, 0, 1, 3);
//    chart = new QChart();
//    chart->createDefaultAxes();
//    chart->setTheme(QChart::ChartThemeDark);
//    chart->legend()->setAlignment(Qt::AlignRight);
//    m_vel_chart = new QChartView(chart);
//    ui->PlanningLayout->addWidget(m_vel_chart, 5, 0, 1, 3);
//#endif
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

void MainWindow::on_TestPTPButton_clicked()
{
    double Bound = 180.0 * 2.0 / 3.0;
    auto ptr_left = dynamic_cast<GComponent::KUKA_IIWA_MODEL*>(GComponent::ModelManager::getInstance().GetModelByName("kuka_iiwa_robot_0"));
    //auto ptr_left = ui->mainArmView->getLeftRobot();
    if (ptr_left) {
        array<double, 7> datas = GenUniformRandom<double, 7>(-Bound, Bound);
        PTPMotion ptp(vector<double>{datas.begin(), datas.end()});
        auto&& [tot, PosFun, VelFun] = ptp.GetCurvesFunction(ptr_left, 20, 5);

        auto joints =  ptr_left->GetComponet<JointGroupComponent>("JointGroupComponent");
        //joints->
        Plot(m_chart->chart(), tot, PosFun, "θ_", "关节曲线图");
        Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");
    }
}

void MainWindow::on_TestLINMotion_clicked()
{
#ifdef __TEST__PLANNING
    auto ptr_left = dynamic_cast<GComponent::KUKA_IIWA_MODEL*>(GComponent::ModelManager::getInstance().GetModelByName("kuka_iiwa_robot_0"));
    if (ptr_left) {
        IIWAThetas  thetas = GComponent::GenUniformRandom<double, 7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
        SE3d        T_ini = ptr_left->ForwardKinematic();
        SE3d        T_goal = ptr_left->ForwardKinematic(thetas);

        LinMotion lin(T_goal);
        auto&& [tot, PosFun, VelFun] = lin.GetCurvesFunction(ptr_left, 0.05, 0.05);

        vec3d EigenPos_ini = T_ini.block(0, 3, 3, 1);
        vec3d EigenPos_end = T_goal.block(0, 3, 3, 1);

        Mat4 base = ptr_left->getModelMatrix();

        /*glm::vec3 pos_goal_affine = Conversion::fromVec3d(EigenPos_end);
        vec3 pos_goal = base * glm::vec4(pos_goal_affine, 1.0f);
        glm::vec3 pos_ini_affine = Conversion::fromVec3d(EigenPos_ini);
        vec3 pos_ini = base * glm::vec4(pos_ini_affine, 1.0f);
        auto pthetas = PosFun(tot);
        std::for_each(pthetas.begin(), pthetas.end(), [](auto&& val) {val = DegreeToRadius(val); });*/

        Plot(m_chart->chart(), tot, PosFun, "θ_", "关节曲线图");
        Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");
    }
#endif
}

void MainWindow::on_TestCircMotion_clicked()
{
#ifdef __TEST__PLANNING
    auto ptr_left = dynamic_cast<GComponent::KUKA_IIWA_MODEL*>(GComponent::ModelManager::getInstance().GetModelByName("kuka_iiwa_robot_0"));
    if (ptr_left) {
        IIWAThetas thetas = GComponent::GenUniformRandom<double, 7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
        IIWAThetas thetasmid = GComponent::GenUniformRandom<double, 7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);

        SE3d  T_ini = ptr_left->ForwardKinematic(),
            T_goal = ptr_left->ForwardKinematic(thetas),
            middle = ptr_left->ForwardKinematic(thetasmid);
        vec3d pos_mid = middle.block(0, 3, 3, 1);

        CircMotion circ(T_goal, pos_mid);

        auto&& [tot, PosFun, VelFun] = circ.GetCurvesFunction(ptr_left, 0.05, 0.05);

        //ui->mainArmView->LeftArmMoveSlot(PosFun, tot);

        Plot(m_chart->chart(), tot, PosFun, "θ_", "关节曲线图");
        Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");

        //ui->mainArmView->clearSimplexModel();        
    }
#endif
}

void MainWindow::on_TestSPLMotion_clicked()
{
#ifdef __TEST__PLANNING
    vector<vec3d> posMidList;
    auto ptr_left = dynamic_cast<GComponent::KUKA_IIWA_MODEL*>(GComponent::ModelManager::getInstance().GetModelByName("kuka_iiwa_robot_0"));
    if (ptr_left) {
        IIWAThetas thetas = GComponent::GenUniformRandom<double, 7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
        SE3d  T_goal = ptr_left->ForwardKinematic(thetas),
            T_ini = ptr_left->ForwardKinematic();

        vec3d pos_end = T_goal.block(0, 3, 3, 1),
            pos_ini = T_ini.block(0, 3, 3, 1);
        auto thetaSS = GenUniformRandoms<double, 2, 7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
        for (auto& theta : thetaSS)
        {
            SE3d T_cur = ptr_left->ForwardKinematic(theta);
            posMidList.emplace_back(T_cur.block(0, 3, 3, 1));
        }

        SPlineMotion splin(T_goal, posMidList);
        auto&& [tot, PosFun, VelFun] = splin.GetCurvesFunction(ptr_left, 0.05, 0.05);

        //ui->mainArmView->LeftArmMoveSlot(PosFun, tot);

        Plot(m_chart->chart(), tot, PosFun, "θ_", "关节曲线图");
        Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");
    }
#endif
}

void MainWindow::on_TestGPMMotion_clicked()
{
#ifdef __TEST__PLANNING

    auto ptr_left = dynamic_cast<GComponent::KUKA_IIWA_MODEL*>(GComponent::ModelManager::getInstance().GetModelByName("kuka_iiwa_robot_0"));
    if (ptr_left) {
        IIWAThetas  thetas = GComponent::GenUniformRandom<double, 7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
        SE3d        T_ini = ptr_left->ForwardKinematic();
        SE3d        T_goal = ptr_left->ForwardKinematic(thetas);

        static vec3d obst = vec3d(0.25f, 0.25f, 0.8f);
        LinMotion lin(T_goal);
        lin.addObstacle("obst", std::make_pair(obst, 0.2f));
        auto&& [tot, PosFun, VelFun] = lin.GetAvoidObstCurvesFunction(ptr_left, 0.02, 0.05, 0.05);

        //ui->mainArmView->LeftArmMoveSlot(PosFun, tot);

        Plot(m_chart->chart(), tot, PosFun, "θ_", "关节曲线图");
        Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");
    }
#endif
}

void MainWindow::on_TestWLNMotion_clicked()
{
    auto ptr_left = dynamic_cast<GComponent::KUKA_IIWA_MODEL*>(GComponent::ModelManager::getInstance().GetModelByName("kuka_iiwa_robot_0"));
    if (ptr_left) {
        IIWAThetas  thetas = GComponent::GenUniformRandom<double, 7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
        SE3d        T_ini = ptr_left->ForwardKinematic();
        SE3d        T_goal = ptr_left->ForwardKinematic(thetas);

        LinMotion lin(T_goal);
        auto&& [tot, PosFun, VelFun] = lin.GetAvoidLimitationCurvesFunction(ptr_left, 0.05, 0.05);

        //ui->mainArmView->LeftArmMoveSlot(PosFun, tot);

        Plot(m_chart->chart(), tot, PosFun, "θ_", "关节曲线图");
        Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");
    }
}

void MainWindow::on_TestZEROMotion_clicked()
{
    auto ptr_left = dynamic_cast<GComponent::KUKA_IIWA_MODEL*>(GComponent::ModelManager::getInstance().GetModelByName("kuka_iiwa_robot_0"));
    if (ptr_left) {
        double tot = 10.0;

        auto checkers = ptr_left->GetCollisionPoints(ptr_left->GetThetas());
        if (checkers.empty()) {
            ptr_left->AddCheckPoint(0, std::make_pair(vec3d(0, 0, 0.360), 0.0));
            ptr_left->AddCheckPoint(1, std::make_pair(vec3d(0, 0, 0.550), 0.0));
            ptr_left->AddCheckPoint(2, std::make_pair(vec3d(0, 0, 0.780), 1.0));
            ptr_left->AddCheckPoint(3, std::make_pair(vec3d(0, 0, 0.950), 0.0));
            ptr_left->AddCheckPoint(5, std::make_pair(vec3d(0, 0, 1.180), 0.0));
        }
        vec3d pobst = vec3d(0.25f, 0.25f, 0.8f);
        auto PosFunction = [ptr_left = ptr_left, pobst, period = 0.0](double t)mutable{
            auto thetas = ptr_left->GetThetas();
            auto mat = ptr_left->NullSpaceMatrix();
            auto grad = ptr_left->GetCollisionGrad(vector{ std::make_pair(pobst, 0.25) }, thetas);
            Matrix<double, 7, 1> delta = mat * grad;

            while (delta.norm() > 0.02)
            {
                delta *= 0.5f;
            }

            double oriVal = ptr_left->GetCollisionVal(vector{ std::make_pair(pobst, 0.25) }, thetas);
            if (delta.transpose() * grad >= 0.0)
                for (int i = 0; i < 7; ++i)
                {
                    thetas[i] += delta(i, 0);
                }
            double curVal = ptr_left->GetCollisionVal(vector{ std::make_pair(pobst, 0.25) }, thetas);
            vector<double> ret_val(thetas.cbegin(), thetas.cend());
            if (oriVal > curVal)
            {
                IIWAThetas theta = ptr_left->GetThetas();
                ret_val.assign(theta.begin(), theta.end());
            }

            std::for_each(ret_val.begin(), ret_val.end(), [](auto&& val) {
                val = ToStandarAngle(val);
                val = RadiusToDegree(val);
                });

            return ret_val;
        };        
        Plot(m_chart->chart(), tot, PosFunction, "θ_", "关节曲线图");
        //ui->mainArmView->LeftArmMoveSlot(PosFunction, tot);
        //ui->mainArmView->addSimplexModel(    "Obst", make_unique<GBall>(obst, 0.25, 20));
    }
}

void MainWindow::on_TestTightCoord_clicked()
{
    KUKA_IIWA_MODEL
            *ptr_left  = dynamic_cast<GComponent::KUKA_IIWA_MODEL*>(GComponent::ModelManager::getInstance().GetModelByName("kuka_iiwa_robot_0")),
            *ptr_right = dynamic_cast<GComponent::KUKA_IIWA_MODEL*>(GComponent::ModelManager::getInstance().GetModelByName("kuka_iiwa_robot_1"));
    if (ptr_left && ptr_right) {
        Vector3d goal_eigen = Vector3d(0.5, GenUniformRandom<double, 1>(-0.2, 0.2)[0], GenUniformRandom<double, 1>(1.3, 1.8)[0]);
        SE3d T_goal = SE3d::Identity();
        T_goal.block(0, 3, 3, 1) = goal_eigen;
        T_goal.block(0, 0, 3, 3) = Roderigues(Vector3d(0.0, 1.0, 0.0), GenUniformRandom<double, 1>(-1.0, 1.0)[0]);
        SyncDualLineMotion dual_line_motion(T_goal);
        SE3d left_bias = SE3d::Identity(),
            right_bias = SE3d::Identity();
        left_bias.block(0, 0, 3, 3) << 1.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 1.0, 0.0;
        right_bias.block(0, 0, 3, 3) << 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, -1.0, 0.0;
        auto&& [tot, pos_func] = dual_line_motion.GetCurvesFunction(ptr_left, ptr_right, left_bias, right_bias);

        std::cout << "time_total=:" << tot << std::endl;

        //ui->mainArmView->DualArmMoveSlot(pos_func, tot);
    }
}

void MainWindow::ConnectionInit()
{
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

void MainWindow::Plot(QChart* chart, double tot, const std::function<vector<double>(double)>& Fun, const std::string& legend, const std::string& title)
{
    const int N = Fun(0).size();
    DataTable myDataLists(N);
    double maxVal = 0.0;
    /* Caculated Y Value */
    for(double t = 0.0; t < tot + 2.0f; t += 0.1f)
    {
        auto values = Fun(t);
        for(int i = 0; i < N; ++i)
        {
            maxVal = std::max(std::abs(values[i]), maxVal);
            QPointF value(t, values[i]);
            myDataLists[i] <<  Data(value, QString::number(t));
        }
    }
    maxVal += 0.1 * maxVal;
    chart->removeAllSeries();
    int Index =0;
    for(auto & datas: myDataLists)
    {
        QSplineSeries * series = new QSplineSeries(chart);
        for(const Data& data : datas)
        {
            series->append(data.first);
        }
        series->setName( QString::fromStdString(legend) + QString::number(++Index));
        chart->addSeries(series);
    }

    chart->setTitle(QString::fromStdString(title));
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setRange(0, tot + 2.0);
    chart->axes(Qt::Vertical).first()->setRange(-maxVal, maxVal);
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

void MainWindow::CheckSelected()
{
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
        ui->trans_x_edit->setText(QString::number(trans.x(), 10, 4));
        ui->trans_y_edit->setText(QString::number(trans.y(), 10, 4));
        ui->trans_z_edit->setText(QString::number(trans.z(), 10, 4));
        Vec3 rot = selected_obj_ptr->getRotGlobal();
        ui->rot_x_edit->setText(QString::number(rot.x(), 10, 4));
        ui->rot_y_edit->setText(QString::number(rot.y(), 10, 4));
        ui->rot_z_edit->setText(QString::number(rot.z(), 10, 4));
        Vec3 scale = selected_obj_ptr->getScale();
        ui->scale_x_edit->setText(QString::number(scale.x(), 10, 4));
        ui->scale_y_edit->setText(QString::number(scale.y(), 10, 4));
        ui->scale_z_edit->setText(QString::number(scale.z(), 10, 4));

        if (last_ptr != selected_obj_ptr) {                
            for (auto& component : selected_obj_ptr->GetComponents()) {
                const string_view& type_name = component->GetTypeName();
                ui->componentstoolbox->addItem(ComponentUIFactory::Create(*component), type_name.data());
            }
        }
    }
    last_ptr = selected_obj_ptr;
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

