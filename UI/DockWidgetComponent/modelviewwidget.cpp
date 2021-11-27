#include "modelviewwidget.h"
#include "ui_modelviewwidget.h"

ModelViewWidget::ModelViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModelViewWidget)
{
    ui->setupUi(this);
}

ModelViewWidget::~ModelViewWidget()
{
    delete ui;
}
