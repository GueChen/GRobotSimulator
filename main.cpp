#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setToolTip("仿真平台");
    w.show();
    return a.exec();
}
