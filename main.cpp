#include "mainwindow.h"
#include "Tooler/conversion.h"
#include <QApplication>
#include <ranges>
#include <iostream>
#include <eigen3/Eigen/Dense>

int main(int argc, char *argv[])
{
    Conversion::ConvertMat3 << -1., 0., 0., 0., 0., 1., 0., 1., 0.;
    Conversion::ConvertMat4 << -1., 0., 0., 0., 0., 0., 1., 0., 0., 1., 0., 0., 0., 0., 0., 1.;
    //Conversion::ConvertMat3 << 0., 1., 0., 0., 0., 1., 1., 0., 0.;
    //Conversion::ConvertMat4 << 0., 1., 0., 0.,  0., 0., 1., 0., 1., 0., 0., 0., 0., 0., 0., 1.;
    QApplication a(argc, argv);
    MainWindow w;
    w.setToolTip("Welcome");
    w.setWindowIconText("机械臂规划仿真软件");
    w.show();
    return a.exec();
}
