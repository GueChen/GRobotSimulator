#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

using std::unique_ptr;
using std::vector;
using std::make_unique;
using UPD = unique_ptr<QDockWidget>;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_modelViewShow_triggered(bool checked);

private:
    Ui::MainWindow *ui;
    vector<UPD> dockWidgets;
};
#endif // MAINWINDOW_H
