#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <unordered_map>
#include <memory>

using std::unique_ptr;
using std::vector;
using std::make_unique;
using UPD = unique_ptr<QDockWidget>;
using Map_StrUPD = std::unordered_map<const std::string, UPD>;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QChartView;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_TestPTPButton_clicked();

private:
    Ui::MainWindow *ui;
    QChartView * m_chart;

};
#endif // MAINWINDOW_H
