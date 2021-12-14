#include "mainwindow.h"
#include <QApplication>
#include <ranges>
#include <iostream>
#include <eigen3/Eigen/Dense>
#include <GComponent/GGeometry.hpp>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setToolTip("Welcome");
    w.show();
    return a.exec();
}
