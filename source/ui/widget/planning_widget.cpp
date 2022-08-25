#include "ui/widget/planning_widget.h"

#include "ui_ptp_motion_planning_widget.h"

#include <QtWidgets/QComboBox>

#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG
namespace GComponent{
PlanningWidget::PlanningWidget(QWidget* parent):
	QWidget(parent),
	ptp_ui_(new Ui::PTPMotionWidget),
	ptp_widget_    (new QWidget(this)),
	line_widget_   (new QWidget(this)),
	circle_widget_ (new QWidget(this)),
	spline_widget_ (new QWidget(this)),
	dual_widget_   (new QWidget(this))
{
	ptp_ui_->setupUi(ptp_widget_);

	widget_tbl_.emplace(QString("PTP"),     ptp_widget_);
	widget_tbl_.emplace(QString("Line"),    line_widget_);
	widget_tbl_.emplace(QString("Circle"),  circle_widget_);
	widget_tbl_.emplace(QString("Spline"),  spline_widget_);
	widget_tbl_.emplace(QString("Dual"),	dual_widget_);

	connect(ptp_ui_->target_combo, &QComboBox::textActivated,
			this,				   &PlanningWidget::PTPTargetSetting);
}

QWidget* PlanningWidget::GetWidget(const QString& wiget_name) const
{
	auto iter = widget_tbl_.find(wiget_name);
	if (iter == widget_tbl_.end()) return nullptr;
	return iter->second;
}

std::vector<QWidget*> PlanningWidget::GetWidgets() const
{
	return {ptp_widget_, line_widget_, circle_widget_, spline_widget_, dual_widget_};
}

PlanningWidget::~PlanningWidget()
{
	delete ptp_widget_;
	delete ptp_ui_;		
}

/*_______________________________PRIVATE SLOT METHODS______________________________________________*/
void PlanningWidget::PTPTargetSetting(const QString& target_type)
{
	std::cout << target_type.toStdString() << std::endl;
	ptp_ui_->data_stack_widget->setCurrentIndex(target_type == "Joint Space" ? 0 : 1);
}

}