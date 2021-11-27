#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QQmlEngine>
#include <QQmlContext>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->menu->setStyleSheet("color: rgb(255, 125, 255)");
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    ui->quickWidget->setAttribute(Qt::WA_AlwaysStackOnTop, true);
    ui->quickWidget->setClearColor(Qt::transparent);

    ui->quickWidget->engine()->rootContext()->setContextProperty("robot", ui->ArmWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_modelViewShow_triggered(bool show)
{

}

