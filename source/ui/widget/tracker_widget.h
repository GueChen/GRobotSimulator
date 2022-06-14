#ifndef _TRACKER_WIDGET_H
#define _TRACKER_WIDGET_H

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class TrackerComponentUI; }
QT_END_NAMESPACE

namespace GComponent {
class TrackerComponentWidget :public QWidget {
	Q_OBJECT
public:
	explicit TrackerComponentWidget(QWidget* parent = nullptr);
	~TrackerComponentWidget();

public slots:
	void SetButtonChecked(int index);
	void SetGoal(const QString& goal_name);

	void AddTrackables(const QList<QString>& trackables);
	void RemoveTrackable(const QString& trackable_name);

	void RemoveTracer(const QString& tracer_name);
	void AddTracer(const QString& tracer_name);
	void AddTracers(const QList<QString>& tracers);

protected slots:
	void on_sleeping_button_clicked();
	void on_tracking_button_clicked();
	void on_target_selector_currentTextChanged(const QString& goal_name);

signals:
	void StateChangedRequest(int idx);
	void TragetSelectedRequest(const QString& goal_name);

private:
	Ui::TrackerComponentUI* ui_ptr_;

};
}

#endif // !_TRACKER_WIDGET_H

