/**
 *  @file  	planningdialog.h
 *  @brief 	planning dialog responsible for interact with worker.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 19th, 2022
 **/
#ifndef __PLANNING_DIALOG_H
#define __PLANNING_DIALOG_H

#include <QtWidgets/QDialog>

#include <memory>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class PlanningDialog;
}
QT_END_NAMESPACE

class QTableWidget;

namespace GComponent {

class PlanningDialog:public QDialog {
	Q_OBJECT
public:
	explicit PlanningDialog(QWidget* parent = nullptr);
	~PlanningDialog();

signals:
	void RequestPTPMotionJSpace(const QString& obj_name, 
								float max_vel,				  float max_acc, 								
								std::vector<float> target_joints);
	void RequestPTPMotionCSpace(const QString& obj_name, 
								float max_vel,				  float max_acc, 
								std::vector<float> target_descarte);
	void RequestLineMotion     (const QString& obj_name, 
								float max_vel,				  float max_acc, 
								float max_ang_vel,			  float max_ang_acc,
								std::vector<float> target_descarte);
	void RequestCircleMotion   (const QString& obj_name, 
								float max_vel,				  float max_acc, 
								float max_ang_vel,			  float max_ang_acc,
								std::vector<float> target_descarte, 
								std::vector<float> waypoint);
	void RequestSplineMotion   (const QString& obj_name, 
								float max_vel,				  float max_acc, 
								float max_ang_vel,			  float max_ang_acc,
								std::vector<float> target_descarte, 
								std::vector<std::vector<float>> waypoints);
	void RequestKeeperMotion   (const QString& obj_name,
								float time,
								std::vector<float> target_descarte);
	void RequestDualSyncLineMotion 
							   (const std::vector<QString>& obj_names, 
								std::vector<float> max_vels,  std::vector<float> max_accs, 
								std::vector<float> target,   
								std::vector<std::vector<float>> bias);
	void RequestDualSyncCircleMotion
							   (const std::vector<QString>& obj_names, 
								std::vector<float> max_vels,  std::vector<float> max_accs, 
								std::vector<float> target,    
								std::vector<std::vector<float>> bias,
								std::vector<float> waypoint);
	void RequestChangeCurrentTaskStatus
							   (const std::vector<QString>& obj_names, int status);

private:
// Process Related Methods
	void PTPMotionExecution();
	void PTPJMotionExecution(const QString& obj_name, float max_vel, float max_acc);
	void PTPCMotionExecution(const QString& obj_name, float max_vel, float max_acc);
	void LineMotionExecution();
	void CircleMotionExecution();
	void SplineMotionExecution();
	void KeeperMotionExecution();
	void DualMotionExecution();
	void DualSyncMotionExecution();	
	void DualAsyncMotionExecution();

// Data Accessor Helper Methods
	std::vector<float> 
		GetOneColFromTable(QTableWidget* tbl, int col_idx);
	std::vector<float>
		GetOneRowFromTable(QTableWidget* tbl, int row_idx);
	std::vector<std::vector<float>>
		GetDatasFromTable(QTableWidget* tbl);

// Status change helper method
	std::vector<QString> GetCurrentObjName() const;

private slots:
	void on_execution_button_clicked();
	void on_cancel_button_clicked();
	void on_stop_button_clicked();
	void on_pause_button_clicked();
	void on_resume_button_clicked();
	void on_spline_waypoints_add_button_clicked();
	void on_spline_waypoints_del_button_clicked();
	void on_spline_waypoints_clear_button_clicked();
	void on_sync_type_combo_activated(int idx);

private:
	Ui::PlanningDialog*		    ui_ptr_				 = nullptr;
};

} // !namespace GComponent

#endif // !__PLANNING_DIALOG_H
