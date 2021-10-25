#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UI/animdockwidget.h"
#include "UI/modelviewdockwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    dockWidgets.push_back(make_unique<AnimDockWidget>());
    dockWidgets.push_back(make_unique<ModelViewDockWidget>());
    for (auto & dockWidget: dockWidgets)
    {
        addDockWidget(Qt::LeftDockWidgetArea, dockWidget.get());
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::on_modelViewShow_triggered(bool show)
{
    if(show)
    {
        dockWidgets[1]->show();
    }
    else
    {
        dockWidgets[1]->hide();
    }
}

