#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <UI/Dialog/PathPlanningSettingDialog.h>

#include <unordered_map>
#include <functional>
#include <memory>


using std::unique_ptr;
using std::vector;
using std::make_unique;
using UPD = unique_ptr<QDockWidget>;
using Map_StrUPD = std::unordered_map<const std::string, UPD>;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QChartView;
class QChart;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_TestPTPButton_clicked();

    void on_TestLINMotion_clicked();

    void on_TestCircMotion_clicked();

    void on_TestSPLMotion_clicked();

    void on_TestGPMMotion_clicked();

    void on_TestZEROMotion_clicked();

    void on_TestWLNMotion_clicked();

    void on_TestTightCoord_clicked();

private:
    Ui::MainWindow *ui;

    void
    Plot(QChart*, double, const std::function<vector<double>(double)>&, const std::string& legend = " ", const std::string& title = "");
    QChartView * m_chart;
    QChartView * m_vel_chart;
    PathPlanningSettingDialog Dialog;
};
#endif // MAINWINDOW_H
