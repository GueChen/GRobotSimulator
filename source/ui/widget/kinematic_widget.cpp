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

}
