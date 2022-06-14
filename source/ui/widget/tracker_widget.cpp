#include "ui/widget/tracker_widget.h"

#include "ui_tracker_component_ui_widget.h"

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG


namespace GComponent {
TrackerComponentWidget::TrackerComponentWidget(QWidget* parent):
	QWidget(parent),
	ui_ptr_(new Ui::TrackerComponentUI)
{
	ui_ptr_->setupUi(this);
}

TrackerComponentWidget::~TrackerComponentWidget()
{
	delete ui_ptr_;
}

void TrackerComponentWidget::SetButtonChecked(int index)
{
	if (index == 0) {
		ui_ptr_->sleeping_button->setChecked(true);
	}
	else{
		ui_ptr_->tracking_button->setChecked(true);
	}
}

void TrackerComponentWidget::SetGoal(const QString& goal_name)
{
	int row = ui_ptr_->target_selector->findText(goal_name);
	if (-1 != row) {
		ui_ptr_->target_selector->setCurrentIndex(row);
	}
}

void TrackerComponentWidget::AddTrackables(const QList<QString>& trackables)
{
	ui_ptr_->target_selector->addItems(trackables);
}

void TrackerComponentWidget::RemoveTrackable(const QString& trackable_name)
{
	int row =ui_ptr_->target_selector->findText(trackable_name);
	if (-1 != row) {
		ui_ptr_->target_selector->removeItem(row);
	}
}

void TrackerComponentWidget::RemoveTracer(const QString& tracer_name)
{
	QList<QListWidgetItem*> remove_items = ui_ptr_->tracer_list->findItems(tracer_name, Qt::MatchExactly);
	for (auto& item : remove_items) {
		ui_ptr_->tracer_list->removeItemWidget(item);
	}
}

void TrackerComponentWidget::AddTracer(const QString& tracer_name)
{
	ui_ptr_->tracer_list->addItem(tracer_name);
}

void TrackerComponentWidget::AddTracers(const QList<QString>& tracers)
{
	ui_ptr_->tracer_list->addItems(tracers);
}
/*___________________________PROTECTED SLOTS________________________________________________*/
void TrackerComponentWidget::on_sleeping_button_clicked()
{
	emit StateChangedRequest(0);
}

void TrackerComponentWidget::on_tracking_button_clicked()
{
	emit StateChangedRequest(1);
}

void TrackerComponentWidget::on_target_selector_currentTextChanged(const QString& goal_name)
{
	emit TragetSelectedRequest(goal_name);
	std::cout << std::format("{:} goal request selected\n", goal_name.toStdString());
}

}
