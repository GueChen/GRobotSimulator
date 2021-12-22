#include <QTimer>
#include <iostream>
#include "dualarmviewwidget.h"
#include "ui_dualarmviewwidget.h"

#include "UI/OpenGLWidget/DualArmView.h"
#include "Component/Object/Robot/joint.h"

#include "Component/Object/BasicMesh/GBasicMesh"


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

    myTimer = new QTimer(this);
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


template<class _Deriv_Simplex>
void DualArmViewWidget::addSimplexModel(const std::string& name, unique_ptr<_Deriv_Simplex>&& uptrSModel)
{
    ui->armView->addSimplexModel(name, std::move(uptrSModel));
}

template void
DualArmViewWidget::addSimplexModel
(const std::string&, unique_ptr<GComponent::GBall>&&);
template void
DualArmViewWidget::addSimplexModel
(const std::string&, unique_ptr<GComponent::GCurves>&&);
template void
DualArmViewWidget::addSimplexModel
(const std::string&, unique_ptr<GComponent::GLine>&&);


void DualArmViewWidget::clearSimplexModel()
{
    ui->armView->clearSimplexModel();
}

/* ****************************************
 *
 *               Slot Functions
 *                  槽函数
 *
 * ****************************************/
void DualArmViewWidget::LeftArmMoveSlot(JointPosFunc posfunc, double T_upper, double period)
{
    auto InvokerFunc = [=,t = 0.0, this,&timer = this->myTimer]() mutable{

        for(int Index = 0;
            auto & theta : posfunc(t))
        {
            setLeftArmRotation(Index, theta);
            ++Index;
        }

        t += period;
        if(t > T_upper)
        {
            timer->stop();
            myTimer->disconnect(this);
        }
    };

    connect(myTimer, &QTimer::timeout, this, InvokerFunc);
    myTimer->start(period * 1000.0);
}
