#include "mainwindow.h"
#include <QApplication>
#include <QQuickWindow>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);


    w.setToolTip("Welcome");
    w.show();
    return a.exec();
}
