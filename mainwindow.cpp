#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QQmlEngine>
#include <QQmlContext>

#define __TEST__PLANNING

#ifdef __TEST__PLANNING
/// Just for Test Include
#include "Component/Object/Motion/GMotion"
#include "Component/Object/BasicMesh/GBasicMesh"

#include "Component/Object/Robot/kuka_iiwa_model.h"
#include <QtWidgets/QGridLayout>
#include <QWidget>
#include <QtCharts/QLegend>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QBarCategoryAxis>
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

#ifdef __TEST__PLANNING
    QChart * chart = new QChart();
    chart->createDefaultAxes();
    chart->setTheme(QChart::ChartThemeDark);
    chart->legend()->setAlignment(Qt::AlignRight);
    m_chart = new QChartView(chart);
    ui->PlanningLayout->addWidget(m_chart, 2, 0, 1, 3);
    chart = new QChart();
    chart->createDefaultAxes();
    chart->setTheme(QChart::ChartThemeDark);
    chart->legend()->setAlignment(Qt::AlignRight);
    m_vel_chart = new QChartView(chart);
    ui->PlanningLayout->addWidget(m_vel_chart, 3, 0, 1, 3);
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
    SE3d        T_ini    = ptr_left->FowardKinematic();
    SE3d        T_goal   = ptr_left->FowardKinematic(thetas);

    LinMotion lin(T_goal);
    auto && [tot, PosFun, VelFun] = lin.GetCurvesFunction(ptr_left, 0.05, 0.05);

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

    auto LineFun = GetScrewLineFunction(T_ini, T_goal);
    for(double t = 0.0; t < 1.0f - 1e-5; t +=0.1f)
    {
        vec3d EigenPos = ExpMapping(LineFun(t)).block(0, 3, 3, 1);
        glm::vec4 pos_cur_affine = glm::vec4(-EigenPos.x(), EigenPos.z(), EigenPos.y(), 1.0f);
        vec3 pos_cur = base * pos_cur_affine;
        int index = t * 10;
        ui->mainArmView->addSimplexModel(std::string("t" + std::to_string(index)), make_unique<GBall>(pos_cur, 0.03f, 5));

    }
    ui->mainArmView->addSimplexModel("Goal_end", make_unique<GBall>(pos_goal, 0.01f, 5));
    ui->mainArmView->addSimplexModel("Goal_ini", make_unique<GBall>(pos_ini,  0.01f, 5));
    ui->mainArmView->addSimplexModel(    "Line", make_unique<GLine>(pos_ini,  pos_goal));
#endif
}

void MainWindow::on_TestCircMotion_clicked()
{
#ifdef __TEST__PLANNING
    auto ptr_left = ui->mainArmView->getLeftRobot();
    IIWAThetas thetas = GComponent::GenUniformRandom<7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);
    IIWAThetas thetasmid = GComponent::GenUniformRandom<7>(-3.1415926 * 2.0 / 3.0, 3.1415926 * 2.0 / 3.0);

    SE3d  T_ini    = ptr_left->FowardKinematic(),
          T_goal   = ptr_left->FowardKinematic(thetas),
          middle   = ptr_left->FowardKinematic(thetasmid);
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
    vec3  pos_ini   = base * glm::vec4(-pos_ini_eigen.x(), pos_ini_eigen.z(), pos_ini_eigen.y(), 1.0f),
          pos_end   = base * glm::vec4(-pos_end_eigen.x(), pos_end_eigen.z(), pos_end_eigen.y(), 1.0f),
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
    SE3d  T_goal   = ptr_left->FowardKinematic(thetas),
          T_ini    = ptr_left->FowardKinematic();

    vec3d pos_end  = T_goal.block(0, 3, 3, 1),
          pos_ini  =  T_ini.block(0, 3, 3, 1);
    auto thetaSS = GenUniformRandoms<5, 7>(-3.1415926 * 2.0 / 6.0, 3.1415926 * 2.0 / 6.0);
    for(auto & theta: thetaSS)
    {
        SE3d T_cur = ptr_left->FowardKinematic(theta);
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
