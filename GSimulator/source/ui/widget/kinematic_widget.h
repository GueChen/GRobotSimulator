/**
 *  @file  	kinematic_widget.h
 *  @brief 	This class is specific ui to fit kinematic component.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	June 13th, 2022
 **/
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

public slots:
	void SetJointCount(int idx);
	void SetPrecision(float min_residual);
	void SetIKSolver(int solver_idx);
	void SetMaxIterations(int max_iterations);
	void SetStepDecay(float decay_val);

protected slots:
	void on_solver_combo_box_currentIndexChanged(int idx);

signals:
	void IKSolverSwitchRequest(int idx);
	void PrecisionCountSettedRequest(int count);
	void MaxIterationsSettedRequest(int max_iterations);
	void StepDecaySettedRequest(float decay_val);

private:
	Ui::KinematicComponentUI* ui_ptr_ = nullptr;
};
}

#endif // _KINEMATIC_WIDGET_H
