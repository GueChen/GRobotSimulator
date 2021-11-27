#include "dualarmviewwidget.h"
#include "ui_dualarmviewwidget.h"
#include "Component/Object/Robot/joint.h"

DualArmViewWidget::DualArmViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DualArmViewWidget)
{
    ui->setupUi(this);
    auto [_lJoints, _rJoints] = ui->armView->getJoints();
    lJoints = _lJoints;
    rJoints = _rJoints;
}

DualArmViewWidget::~DualArmViewWidget()
{
    delete ui;
}

void DualArmViewWidget::setLeftArmRotation(QVariant idx, QVariant value)
{
    (dynamic_cast<GComponent::Revolute*>
                (lJoints[idx.toInt()]))->Rotate(value.toDouble());
}
