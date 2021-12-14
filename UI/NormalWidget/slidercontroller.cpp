#include "slidercontroller.h"
#include "ui_slidercontroller.h"
#include <QTabWidget>
#include <QTabBar>
SliderController::SliderController(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SliderController)
{
    ui->setupUi(this);
    ui->armTab->tabBar()->setStyleSheet("background-color:rgb(40,40,40);");
}

SliderController::~SliderController()
{
    delete ui;
}
