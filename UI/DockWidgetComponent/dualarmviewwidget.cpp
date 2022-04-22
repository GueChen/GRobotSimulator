#include "dualarmviewwidget.h"
#include "ui_dualarmviewwidget.h"

#include "GComponent/GNumerical.hpp"

#include "Base/editortreemodel.h"
#include "Component/Object/Robot/joint.h"
#include "Component/Object/BasicMesh/GBasicMesh"
#include "UI/OpenGLWidget/DualArmView.h"
#include "Tooler/conversion.h"

#include <QTimer>
#include <QFile>
#include <iostream>

//FiXME:测试完后删除耦合
/// For Test
#include "Component/Object/Robot/kuka_iiwa_model.h"

DualArmViewWidget::DualArmViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DualArmViewWidget)
{
    ui->setupUi(this);
    ui->splitView->setStretchFactor(0, 1);
    ui->splitView->setStretchFactor(1, 9);
    QFile file("Resource/tree.ini");
    file.open(QIODevice::ReadOnly);
    treemodel_ = new TreeModel(file.readAll());
    ui->treeView->setModel(treemodel_);

    timer_ = new QTimer(this);
}

DualArmViewWidget::~DualArmViewWidget()
{
    delete ui;
    timer_->stop();
    timer_->disconnect();
    delete timer_;
    delete treemodel_;
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
    auto InvokerFunc = [=, this, t = 0.0, &timer = this->timer_]() mutable{
        auto vecPos = posfunc(t);
        for_each(vecPos.begin(), vecPos.end(), [](auto && theta){
            theta = GComponent::DegreeToRadius(theta);
        });
        GComponent::IIWAThetas thetas{
            vecPos[0], vecPos[1], vecPos[2], vecPos[3],
            vecPos[4], vecPos[5], vecPos[6]};
        getLeftRobot()->Move(thetas);
        auto ptr = this->getLeftRobot();
        auto checkers = ptr->GetCollisionPoints(thetas);
        if(!checkers.empty()){
            int index = 0;
            for(auto & check: checkers)
            {
                glm::vec3 p = Conversion::Vec3Eigen2Glm(check);
                glm::mat4 base = ptr->getModelMatrix();
                p = base * glm::vec4(p, 1.0f);
                this->addSimplexModel("Checker" + std::to_string(index++), std::make_unique<GComponent::GBall>(p, 0.1, 20));
            }
        }
        t += period;
        if(t > T_upper)
        {
            timer->stop();
            timer_->disconnect(this);
        }
    };

    connect(timer_, &QTimer::timeout, this, InvokerFunc);
    timer_->start(period * 100.0);
}

void DualArmViewWidget::DualArmMoveSlot(DualJointsPosFunc posfunc, double T_upper, double period){
    auto InvokerFunc = [=, this, t= 0.0, &timer = this->timer_]()mutable {
        auto&& [left_pos_vec, right_pos_vec] = posfunc(t);

        for(auto && pos: left_pos_vec){
            pos = GComponent::DegreeToRadius(pos);
        }
        for(auto && pos: right_pos_vec){
            pos = GComponent::DegreeToRadius(pos);
        }
        GComponent::IIWAThetas left_thetas{
            left_pos_vec[0], left_pos_vec[1], left_pos_vec[2], left_pos_vec[3],
            left_pos_vec[4], left_pos_vec[5], left_pos_vec[6]},
                               right_thetas{
            right_pos_vec[0], right_pos_vec[1], right_pos_vec[2], right_pos_vec[3],
            right_pos_vec[4], right_pos_vec[5], right_pos_vec[6]};

        getLeftRobot()->Move(left_thetas);
        getRightRobot()->Move(right_thetas);

        t += period;
        if(t > T_upper)
        {
            timer->stop();
            timer_->disconnect(this);
        }
    };
    connect(timer_, &QTimer::timeout, this, InvokerFunc);
    timer_->start(period * 100.0);
}
