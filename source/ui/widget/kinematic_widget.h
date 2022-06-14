#ifndef _KINEMATIC_WIDGET_H
#define _KINEMATIC_WIDGET_H

#include <QtWidgets/Qwidget>

QT_BEGIN_NAMESPACE
namespace Ui { class KinematicComponentUI; }
QT_END_NAMESPACE

namespace GComponent{
class KinematicComponentWidget : public QWidget {
	Q_OBJECT
public:
	explicit KinematicComponentWidget(QWidget* parent = nullptr);
	~KinematicComponentWidget();

private:
	Ui::KinematicComponentUI* ui_ptr_ = nullptr;
};
}

#endif // _KINEMATIC_WIDGET_H
