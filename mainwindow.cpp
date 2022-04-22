#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QQmlEngine>
#include <QQmlContext>
#include <QTabBar>

#define __TEST__PLANNING

#ifdef __TEST__PLANNING
/// Just for Test Include
#include "Component/Object/Motion/GMotion"
#include "Component/Object/BasicMesh/GBasicMesh"

#include "Component/Object/Robot/kuka_iiwa_model.h"

#include "Tooler/conversion.h"

#include <QtWidgets/QGridLayout>
#include <QWidget>
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
    ui->menu->setStyleSheet("color: rgb(255, 125, 255)");

    ui->ModelObserverDocker->hide();
    ui->DualArmDocker->hide();

    for(auto & ptr_tab: findChildren<QTabBar*>()){
        qDebug() << ptr_tab->objectName();
    }
    connect(this, &MainWindow::tabifiedDockWidgetActivated, this, [=, this](){
        for(auto & ptr_tabwidget_tab : findChildren<QTabBar*>()){
            if(ptr_tabwidget_tab->objectName() == ""){
            ptr_tabwidget_tab->setStyleSheet(
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
    });

#ifdef __TEST__PLANNING
    QChart * chart = new QChart();
    chart->createDefaultAxes();
    chart->setTheme(QChart::ChartThemeDark);
    chart->legend()->setAlignment(Qt::AlignRight);
    m_chart = new QChartView(chart);
    ui->PlanningLayout->addWidget(m_chart, 4, 0, 1, 3);
    chart = new QChart();
    chart->createDefaultAxes();
    chart->setTheme(QChart::ChartThemeDark);
    chart->legend()->setAlignment(Qt::AlignRight);
    m_vel_chart = new QChartView(chart);
    ui->PlanningLayout->addWidget(m_vel_chart, 5, 0, 1, 3);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_TestPTPButton_clicked()
{
    constexpr double Bound = 180.0 * 2.0 / 3.0;
    auto ptr_left = ui->mainArmView->getLeftRobot();
    array<double, 7> datas = GenUniformRandom<7>(-Bound, Bound);
    PTPMotion ptp(vector<double>{datas.begin(), datas.end()});
    auto && [tot, PosFun, VelFun] = ptp.GetCurvesFunction(ptr_left, 20, 5);
    DataList lastData;
    for(auto & val : ptr_left->GetThetas())
    {
        lastData << Data(QPointF(0, val), QString::number(0));
    }
    ui->mainArmView->LeftArmMoveSlot(PosFun, tot);

    Plot(m_chart->chart(), tot, PosFun, "θ_", "关节曲线图");
    Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");
}

void MainWindow::on_TestLINMotion_clicked()
{
#ifdef __TEST__PLANNING
    KUKA_IIWA_MODEL*
                ptr_left = ui->mainArmView->getLeftRobot();
    IIWAThetas  thetas   = GComponent::GenUniformRandom<7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
    SE3d        T_ini    = ptr_left->ForwardKinematic();
    SE3d        T_goal   = ptr_left->ForwardKinematic(thetas);

    LinMotion lin(T_goal);
    auto && [tot, PosFun, VelFun] = lin.GetCurvesFunction(ptr_left, 0.05, 0.05);

    ui->mainArmView->clearSimplexModel();
    vec3d EigenPos_ini = T_ini.block(0, 3, 3, 1);
    vec3d EigenPos_end = T_goal.block(0, 3, 3, 1);

    mat4 base = ptr_left->getModelMatrix();

    glm::vec3 pos_goal_affine = Conversion::Vec3Eigen2Glm(EigenPos_end);
    vec3 pos_goal = base * glm::vec4(pos_goal_affine, 1.0f);
    glm::vec3 pos_ini_affine  = Conversion::Vec3Eigen2Glm(EigenPos_ini);
    vec3 pos_ini  = base * glm::vec4(pos_ini_affine, 1.0f);
    auto pthetas  = PosFun(tot);
    std::for_each(pthetas.begin(), pthetas.end(), [](auto && val){val = DegreeToRadius(val);});

    ui->mainArmView->addSimplexModel("Goal_end", make_unique<GBall>(pos_goal, 0.01f, 5));
    ui->mainArmView->addSimplexModel("Goal_ini", make_unique<GBall>(pos_ini,  0.01f, 5));
    ui->mainArmView->addSimplexModel(    "Line", make_unique<GLine>(pos_ini,  pos_goal));
    ui->mainArmView->LeftArmMoveSlot(PosFun, tot);

    Plot(m_chart->chart(),     tot, PosFun, "θ_", "关节曲线图");
    Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");
#endif
}

void MainWindow::on_TestCircMotion_clicked()
{
#ifdef __TEST__PLANNING
    auto ptr_left = ui->mainArmView->getLeftRobot();
    IIWAThetas thetas = GComponent::GenUniformRandom<7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
    IIWAThetas thetasmid = GComponent::GenUniformRandom<7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);

    SE3d  T_ini    = ptr_left->ForwardKinematic(),
          T_goal   = ptr_left->ForwardKinematic(thetas),
          middle   = ptr_left->ForwardKinematic(thetasmid);
    vec3d pos_mid  = middle.block(0, 3, 3, 1);

    CircMotion circ(T_goal, pos_mid);

    auto && [tot, PosFun, VelFun] = circ.GetCurvesFunction(ptr_left, 0.05, 0.05);

    ui->mainArmView->LeftArmMoveSlot(PosFun, tot);

    Plot(m_chart->chart(), tot, PosFun, "θ_", "关节曲线图");
    Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");

    ui->mainArmView->clearSimplexModel();

    mat4  base = ptr_left->getModelMatrix();
    vec3d pos_ini_eigen = T_ini.block(0, 3, 3, 1),
          pos_end_eigen = T_goal.block(0, 3, 3, 1);
    auto circleFunc = GetCircleFunction(pos_ini_eigen, pos_end_eigen, pos_mid);
    vec3  pos_ini     = base * glm::vec4(-pos_ini_eigen.x(), pos_ini_eigen.z(), pos_ini_eigen.y(), 1.0f),
          pos_end     = base * glm::vec4(-pos_end_eigen.x(), pos_end_eigen.z(), pos_end_eigen.y(), 1.0f),
          pos_mid_glm = base * glm::vec4(-pos_mid.x(), pos_mid.z(), pos_mid.y(), 1.0f);
    vector<vec3> poses;
    for(double t = 0.0; t < 1.0 - 1e-5; t += 0.01)
    {
        vec3d pos_eigen = circleFunc(t);
        vec3 pos_cur = base * glm::vec4(-pos_eigen.x(), pos_eigen.z(), pos_eigen.y(), 1.0f);
        std::cout << "Insert " << t << "\n";
        poses.emplace_back(pos_cur);
    }

    ui->mainArmView->addSimplexModel("Goal_mid", make_unique<GBall>(pos_mid_glm, 0.05f, 5));
    ui->mainArmView->addSimplexModel("Goal_ini", make_unique<GBall>(pos_ini,     0.05f, 5));
    ui->mainArmView->addSimplexModel("Goal_end", make_unique<GBall>(pos_end,     0.05f, 5));
    ui->mainArmView->addSimplexModel("GCurveLine", make_unique<GCurves>(poses));

#endif
}

void MainWindow::on_TestSPLMotion_clicked()
{
#ifdef __TEST__PLANNING
    vector<vec3d> posMidList;
    auto ptr_left = ui->mainArmView->getLeftRobot();
    IIWAThetas thetas = GComponent::GenUniformRandom<7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
    SE3d  T_goal   = ptr_left->ForwardKinematic(thetas),
          T_ini    = ptr_left->ForwardKinematic();

    vec3d pos_end  = T_goal.block(0, 3, 3, 1),
          pos_ini  =  T_ini.block(0, 3, 3, 1);
    auto thetaSS = GenUniformRandoms<2, 7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
    for(auto & theta: thetaSS)
    {
        SE3d T_cur = ptr_left->ForwardKinematic(theta);
        posMidList.emplace_back(T_cur.block(0, 3, 3, 1));
    }

    SPlineMotion splin(T_goal, posMidList);
    auto && [tot, PosFun, VelFun] = splin.GetCurvesFunction(ptr_left, 0.05, 0.05);
    ui->mainArmView->LeftArmMoveSlot(PosFun, tot);

    Plot(m_chart->chart(), tot, PosFun, "θ_", "关节曲线图");
    Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");

    posMidList.insert(posMidList.begin(), pos_ini);
    posMidList.insert(posMidList.end(), pos_end);
    auto CubicSplFun = GetCubicSplineFunction(posMidList);
    vector<vec3> poses;
    mat4  base = ptr_left->getModelMatrix();
    for(double t = 0.0; t < 1.0 - 1e-5; t += 0.01)
    {
        vec3d pos_eigen = CubicSplFun(t);
        vec3 pos_cur = base * glm::vec4(-pos_eigen.x(), pos_eigen.z(), pos_eigen.y(), 1.0f);
        std::cout << "Insert " << t << "\n";
        poses.emplace_back(pos_cur);
    }

    for(int Index = 0;auto & pose: posMidList)
    {
        vec3 pos_cur = base * glm::vec4(-pose.x(), pose.z(), pose.y(), 1.0f);
        ui->mainArmView->addSimplexModel(std::string("t" + std::to_string(Index++)), make_unique<GBall>(pos_cur, 0.03f, 5));
    }
    ui->mainArmView->addSimplexModel("CubicSPLine", make_unique<GCurves>(poses));

#endif
}

void MainWindow::on_TestGPMMotion_clicked()
{
#ifdef __TEST__PLANNING
    Dialog.show();
    auto && [ini, end] = Dialog.GetThetas();

    KUKA_IIWA_MODEL*
                ptr_left = ui->mainArmView->getLeftRobot();

    IIWAThetas  thetas   = GComponent::GenUniformRandom<7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
    SE3d        T_ini    = ptr_left->ForwardKinematic();
    SE3d        T_goal   = ptr_left->ForwardKinematic(thetas);

    static vec3d obst    = vec3d(0.25f, 0.25f, 0.8f);
    LinMotion lin(T_goal);
    lin.addObstacle("obst", std::make_pair(obst, 0.2f));
    auto && [tot, PosFun, VelFun] = lin.GetAvoidObstCurvesFunction(ptr_left, 0.02, 0.05, 0.05);

    ui->mainArmView->LeftArmMoveSlot(PosFun, tot);

    Plot(m_chart->chart(), tot, PosFun, "θ_", "关节曲线图");
    Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");

    ui->mainArmView->clearSimplexModel();
    vec3d EigenPos_ini = T_ini.block(0, 3, 3, 1);
    vec3d EigenPos_end = T_goal.block(0, 3, 3, 1);

    mat4 base = ptr_left->getModelMatrix();
    glm::vec4 pos_goal_affine = glm::vec4(-EigenPos_end.x(), EigenPos_end.z(), EigenPos_end.y(), 1.0f);
    vec3 pos_goal = base * pos_goal_affine;
    glm::vec4 pos_ini_affine  = glm::vec4( -EigenPos_ini.x(), EigenPos_ini.z(), EigenPos_ini.y(), 1.0f);
    vec3 pos_ini  = base * pos_ini_affine;
    vec3 pos_obst = base * glm::vec4(-obst.x(), obst.z(), obst.y(), 1.0f);

    vector<vec3> curves;
    for(double t = 0.0; t < tot - 1e-5; t +=0.1f)
    {
        vector<double> Vthetas = PosFun(t);
        IIWAThetas thetas;

        std::transform(Vthetas.begin(), Vthetas.end(), thetas.begin(),[](auto&& num){return DegreeToRadius(num);});
        vec3d EigenPos = ptr_left->ForwardKinematic(thetas).block(0, 3, 3, 1);

        vec3 pos_cur = base * glm::vec4(-EigenPos.x(), EigenPos.z(), EigenPos.y(), 1.0f);
        curves.emplace_back(pos_cur);
    }

    ui->mainArmView->addSimplexModel("Goal_end", make_unique<GBall>(pos_goal, 0.01f,  5));
    ui->mainArmView->addSimplexModel("Goal_ini", make_unique<GBall>( pos_ini, 0.01f,  5));
    ui->mainArmView->addSimplexModel(    "Obst", make_unique<GBall>(pos_obst, 0.2f, 20));
    ui->mainArmView->addSimplexModel(    "Line", make_unique<GCurves>(curves));

    std::for_each(ini.begin(), ini.end(), [](auto&&val){std::cout << " " << val;});
#endif
}

void MainWindow::on_TestWLNMotion_clicked()
{
    KUKA_IIWA_MODEL*
                ptr_left = ui->mainArmView->getLeftRobot();
    IIWAThetas  thetas   = GComponent::GenUniformRandom<7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
    SE3d        T_ini    = ptr_left->ForwardKinematic();
    SE3d        T_goal   = ptr_left->ForwardKinematic(thetas);

    LinMotion lin(T_goal);
    auto && [tot, PosFun, VelFun] = lin.GetAvoidLimitationCurvesFunction(ptr_left, 0.05, 0.05);

    ui->mainArmView->clearSimplexModel();
    vec3d EigenPos_ini = T_ini.block(0, 3, 3, 1);
    vec3d EigenPos_end = T_goal.block(0, 3, 3, 1);

    vec3 pos_goal = ptr_left->getModelMatrix() * glm::vec4(Conversion::Vec3Eigen2Glm(EigenPos_end), 1.0f);
    vec3 pos_ini  = ptr_left->getModelMatrix() * glm::vec4(Conversion::Vec3Eigen2Glm(EigenPos_ini), 1.0f);
    auto pthetas  = PosFun(tot);
    std::for_each(pthetas.begin(), pthetas.end(), [](auto && val){val = DegreeToRadius(val);});

    ui->mainArmView->addSimplexModel("Goal_end", make_unique<GBall>(pos_goal, 0.01f, 5));
    ui->mainArmView->addSimplexModel("Goal_ini", make_unique<GBall>(pos_ini,  0.01f, 5));
    ui->mainArmView->addSimplexModel(    "Line", make_unique<GLine>(pos_ini,  pos_goal));
    ui->mainArmView->LeftArmMoveSlot(PosFun, tot);

    Plot(m_chart->chart(),     tot, PosFun, "θ_", "关节曲线图");
    Plot(m_vel_chart->chart(), tot, VelFun, "v_", "关节速度曲线图");
}

void MainWindow::on_TestZEROMotion_clicked()
{
    KUKA_IIWA_MODEL * ptr_left = ui->mainArmView->getLeftRobot();
    double tot = 10.0;

    std::cout << "here I'm Press\n";
    static Eigen::Matrix<double, 7, 1> inc = Eigen::MatrixXd::Zero(7, 1);
    if(inc.norm() < 1e-10)
    {
        inc << 0, 0, 0.1, 0, -0.1, -0.1, 0;
    }
    auto checkers = ptr_left->GetCollisionPoints(ptr_left->GetThetas());
    if(checkers.empty()){
        ptr_left->AddCheckPoint(0, std::make_pair(vec3d(0, 0, 0.360), 0.0));
        ptr_left->AddCheckPoint(1, std::make_pair(vec3d(0, 0, 0.550), 0.0));
        ptr_left->AddCheckPoint(2, std::make_pair(vec3d(0, 0, 0.780), 1.0));
        ptr_left->AddCheckPoint(3, std::make_pair(vec3d(0, 0, 0.950), 0.0));
        ptr_left->AddCheckPoint(5, std::make_pair(vec3d(0, 0, 1.180), 0.0));
    }
    vec3d pobst = vec3d(0.25f, 0.25f, 0.8f);
    auto PosFunction = [ptr_left = ptr_left, pobst, period = 0.0, &inc = inc](double t)mutable{
        auto thetas = ptr_left->GetThetas();
        auto mat    = ptr_left->NullSpaceMatrix();
        auto grad   = ptr_left->GetCollisionGrad(vector{std::make_pair(pobst, 0.25)}, thetas);
        Matrix<double, 7, 1> delta = mat * grad;

        while(delta.norm() > 0.02 )
        {
            delta *=0.5f;
        }

        double oriVal   = ptr_left->GetCollisionVal(vector{std::make_pair(pobst, 0.25)}, thetas);
        if(delta.transpose() * grad >= 0.0)
        for(int i = 0; i < 7; ++i)
        {
            thetas[i] += delta(i, 0);
        }
        double curVal = ptr_left->GetCollisionVal(vector{std::make_pair(pobst, 0.25)}, thetas);
        vector<double> ret_val(thetas.cbegin(), thetas.cend());
        if(oriVal > curVal)
        {
            IIWAThetas theta = ptr_left->GetThetas();
            ret_val.assign(theta.begin(), theta.end());
        }

        std::for_each(ret_val.begin(), ret_val.end(), [](auto && val){
            val = ToStandarAngle(val);
            val = RadiusToDegree(val);
        });

        return ret_val;
    };
    mat4 base = ptr_left->getModelMatrix();
    vec3 glmexp = Conversion::Vec3Eigen2Glm(pobst);
    vec3 obst = base * glm::vec4(glmexp, 1.0f);
    ui->mainArmView->LeftArmMoveSlot(PosFunction, tot);
    Plot(m_chart->chart(), tot, PosFunction, "θ_", "关节曲线图");
    ui->mainArmView->addSimplexModel(    "Obst", make_unique<GBall>(obst, 0.25, 20));
}

void MainWindow::on_TestTightCoord_clicked()
{
    KUKA_IIWA_MODEL
            *ptr_left  = ui->mainArmView->getLeftRobot(),
            *ptr_right = ui->mainArmView->getRightRobot();

    Vector3d goal_eigen = Vector3d(GenUniformRandom<1>(-0.2, 0.2)[0], 0.5, GenUniformRandom<1>(1.3, 1.8)[0]);
    SE3d T_goal = SE3d::Identity();
    T_goal.block(0, 3, 3, 1) = goal_eigen;
    T_goal.block(0, 0, 3, 3) = Roderigues(Vector3d(0.0, 1.0, 0.0), GenUniformRandom<1>(-1.0, 1.0)[0]);
    SyncDualLineMotion dual_line_motion(T_goal);
    SE3d left_bias = SE3d::Identity(),
         right_bias= SE3d::Identity();
    left_bias.block(0, 0, 3, 3)  << 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, -1.0, 0.0, 0.0;
    right_bias.block(0, 0, 3, 3) << 0.0, 0.0, -1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0;
    auto && [tot, pos_func] = dual_line_motion.GetCurvesFunction(ptr_left, ptr_right, left_bias, right_bias);

    std::cout << "time_total=:" << tot << std::endl;

    ui->mainArmView->clearSimplexModel();
    //auto && [left_T, right_T] = dual_line_motion.GetTGoalTransforms(ptr_left, ptr_right);
    Vector3d goal_ini_eigen_left = ptr_left->ForwardKinematic().block(0, 3, 3 ,1),
             goal_ini_eigen_right= ptr_right->ForwardKinematic().block(0, 3, 3, 1);
    vec3 pos_goal       = Conversion::Vec3Eigen2Glm(goal_eigen);
    vec3 pos_ini_left   = ptr_left->getModelMatrix() * glm::vec4(Conversion::Vec3Eigen2Glm(goal_ini_eigen_left), 1.0f),
         pos_ini_right  = ptr_right->getModelMatrix()* glm::vec4(Conversion::Vec3Eigen2Glm(goal_ini_eigen_right), 1.0f);

    ui->mainArmView->addSimplexModel("left_traj_Line",  make_unique<GLine>(pos_ini_left, pos_goal, vec3(1.0f), vec3(1.0f)));
    ui->mainArmView->addSimplexModel("right_traj_Line", make_unique<GLine>(pos_ini_right, pos_goal, vec3(1.0f), vec3(1.0f)));
    ui->mainArmView->addSimplexModel("Goal_end_ori", make_unique<GBall>(pos_goal, 0.01f, 5));
    ui->mainArmView->addSimplexModel("ini_end_left", make_unique<GBall>(pos_ini_left, 0.01f, 5));


    ui->mainArmView->DualArmMoveSlot(pos_func, tot);

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


