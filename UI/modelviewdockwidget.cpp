#include <QSplitter>
#include "modelviewdockwidget.h"
#include "ui_modelviewdockwidget.h"

ModelViewDockWidget::ModelViewDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ModelViewDockWidget)
{
    ui->setupUi(this);
    m_spliter = new QSplitter(ui->dockWidgetContents);
    m_spliter->setGeometry(0,0,width(), height());
    m_spliter->addWidget(ui->treeView);
    m_spliter->addWidget(ui->openGLWidget);

}

ModelViewDockWidget::~ModelViewDockWidget()
{
    delete ui;
}

void ModelViewDockWidget::resizeEvent(QResizeEvent *event)
{

    m_spliter->setGeometry(0,0,width(), height());
}
