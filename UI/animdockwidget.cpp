#include "animdockwidget.h"
#include "ui_animdockwidget.h"
#include "myopglwidget.h"
AnimDockWidget::AnimDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::AnimDockWidget)
{
    ui->setupUi(this);
    p_myWindow = std::make_unique<myOpglWidget>(new myOpglWidget);
    ui->gridLayout->addWidget(p_myWindow.get(), 1, 0, 1, 1);
}

AnimDockWidget::~AnimDockWidget()
{
    delete ui;
}

void AnimDockWidget::on_pushButton_clicked()
{
    ((myOpglWidget*)p_myWindow.get())->black = !((myOpglWidget*)p_myWindow.get())->black;
}

