#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "UI/animdockwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    dockWidgets.push_back(make_unique<AnimDockWidget>(new AnimDockWidget));
    for (auto & dockWidget: dockWidgets)
    {
        addDockWidget(Qt::LeftDockWidgetArea, dockWidget.get());
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

