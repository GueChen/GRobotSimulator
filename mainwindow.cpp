#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QQmlEngine>
#include <QQmlContext>

#define __TEST__PLANNING

#ifdef __TEST__PLANNING
/// Just for Test Include
#include "Component/Object/Motion/ptpmotion.h"
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
    chart->setTitle("速度曲线图");
    chart->createDefaultAxes();
    //chart->axes(Qt::Horizontal).first()->setRange(0, 10);
    //chart->axes(Qt::Vertical).first()->setRange(0, 10);
    m_chart = new QChartView(chart);
    ui->PlanningLayout->layout()->addWidget(m_chart);
    //ui->ChartView = chartView;


#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_TestPTPButton_clicked()
{
#ifdef __TEST__PLANNING
    auto ptr_left = ui->mainArmView->getLeftRobot();
    PTPMotion ptp({120, 160, 150, 50, -60, -180, 80});
    auto velFun = ptp.GetCurvesFunction(ptr_left);

    QChart * chart = m_chart->chart();
    DataTable myDataLists(7);
    for(double t = 0.0; t < 45; t += 0.1f)
    {
        auto Values = velFun(t);

        for(int i = 0; i < 7; ++i)
        {
            QPointF value(t, Values[i]);
            myDataLists[i] << Data(value, QString::number(t));
        }
    }

    int Index =0;
    for(auto & datas: myDataLists)
    {
        QSplineSeries * series = new QSplineSeries(chart);
        for(const Data& data : datas)
        {
            series->append(data.first);
        }
        series->setName("θ_" + QString::number(++Index));
        chart->addSeries(series);
    }

    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setRange(0, 40);
    chart->axes(Qt::Vertical).first()->setRange(-6, 6);


#endif
}

