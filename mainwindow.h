#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QComboBox>

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

namespace GComponent{
class UIState;
class GLModelTreeView;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT        
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    GComponent::UIState*         getUIState()       const;
    GComponent::GLModelTreeView* getModelTreeView() const; 
    QComboBox*                   getModelDispaly()  const;

private:
    void ConnectionInit();

    void Plot(QChart*, double, const std::function<vector<double>(double)>&, const std::string& legend = " ", const std::string& title = "");
  
private slots:
    void ReceiveDeltaTime(float delta_time);
    void SetTabifyDockerWidgetQSS(QDockWidget* widget);
    void CheckSelected();

    void on_trans_button_clicked();
    void on_rot_button_clicked();
    void on_scale_button_clicked();

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
    Ui::MainWindow*     ui;
    QTimer*             updated_timer_ptr;
    QChartView*         m_chart;
    QChartView*         m_vel_chart;
   
};
#endif // MAINWINDOW_H
