/**
 *  @file  	planning_widget.h
 *  @brief 	the planning widget to posses intract datas acoording to combo selection.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 19th, 2022
 **/
#ifndef __PLANNING_WIDGET_H
#define __PLANNING_WIDGET_H

#include <QtWidgets/QWidget>

#include <unordered_map>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class PTPMotionWidget;
class LineMotionWidget;
class CircleMotionWidget;
class SplineMotionWidget;
class DualLineMotionWidget;
}
QT_END_NAMESPACE

namespace GComponent {

class PlanningWidget: public QWidget{
	Q_OBJECT
public:
	PlanningWidget(QWidget* parent);
	~PlanningWidget();

	QWidget*			  GetWidget(const QString& wiget_name) const;
	std::vector<QWidget*> GetWidgets()						   const;

private slots:	
	void PTPTargetSetting(const QString& target_type);
	
private:
	std::unordered_map<QString, QWidget*> widget_tbl_;

/*___________________________Widget Wrapper______________________________________*/
	QWidget*	ptp_widget_			= nullptr;
	QWidget*	line_widget_		= nullptr;
	QWidget*	circle_widget_		= nullptr;
	QWidget*	spline_widget_		= nullptr;
	QWidget*	dual_widget_		= nullptr;
/*___________________________UI__________________________________________________*/
	Ui::PTPMotionWidget*		ptp_ui_		= nullptr;
	Ui::LineMotionWidget*		line_ui_	= nullptr;
	Ui::CircleMotionWidget*		circle_ui_	= nullptr;
	Ui::SplineMotionWidget*		spline_ui_  = nullptr;
	Ui::DualLineMotionWidget*	dual_ui_	= nullptr;
};

} // !namespace GComponent

#endif // !__PLANNING_WIDGET_H
