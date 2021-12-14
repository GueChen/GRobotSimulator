#include "datadisplaycanvas.h"
#include "ui_datadisplaycanvas.h"

DataDisplayCanvas::DataDisplayCanvas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataDisplayCanvas)
{
    ui->setupUi(this);
}

DataDisplayCanvas::~DataDisplayCanvas()
{
    delete ui;
}
