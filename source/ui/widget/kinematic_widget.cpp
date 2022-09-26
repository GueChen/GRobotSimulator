#include "ui/widget/kinematic_widget.h"

#include "ui_kinematic_component_ui_widget.h"

namespace GComponent {
KinematicComponentWidget::KinematicComponentWidget(QWidget* parent) :
	QWidget(parent),
	ui_ptr_(new Ui::KinematicComponentUI)
{
	ui_ptr_->setupUi(this);
}

GComponent::KinematicComponentWidget::~KinematicComponentWidget()
{
	delete ui_ptr_;
}

void KinematicComponentWidget::SetPrecision(float min_residual)
{
	ui_ptr_->precesion_val->setText(QString::number(min_residual));
}

void KinematicComponentWidget::SetIKSolver(int solver_idx)
{
	ui_ptr_->solver_combo_box->setCurrentIndex(solver_idx);
}

void KinematicComponentWidget::SetMaxIterations(int max_iterations)
{
	ui_ptr_->max_iterations_val->setText(QString::number(max_iterations));
}

void KinematicComponentWidget::SetStepDecay(float decay_val)
{
	ui_ptr_->decay_val->setText(QString::number(decay_val));
}

void KinematicComponentWidget::SetJointCount(int idx)
{
	ui_ptr_->joint_count_val->setText(QString::number(idx));
}

void KinematicComponentWidget::on_solver_combo_box_currentIndexChanged(int idx)
{
	emit IKSolverSwitchRequest(idx);
}

}
