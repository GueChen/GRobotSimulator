#include "dualarmviewwidget.h"
#include "ui_dualarmviewwidget.h"

#include "UI/OpenGLWidget/DualArmView.h"
#include "Component/Object/Robot/joint.h"

DualArmViewWidget::DualArmViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DualArmViewWidget)
{
    ui->setupUi(this);
    ui->splitView->setStretchFactor(0, 1);
    ui->splitView->setStretchFactor(1, 9);
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

GComponent::KUKA_IIWA_MODEL* DualArmViewWidget::getLeftRobot() const
{
    return ui->armView->getLeftRobot();
}

GComponent::KUKA_IIWA_MODEL* DualArmViewWidget::getRightRobot() const
{
    return ui->armView->getRightRobot();
}
