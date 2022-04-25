#include "mainwindow.h"
#include "function/conversion.h"
#include "engine.h"
#include <QtWidgets/QApplication>
#include <QtCore/QThreadPool>


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

    QThreadPool::globalInstance()->start(&GComponent::Engine::getInstance());

    return a.exec();
}
