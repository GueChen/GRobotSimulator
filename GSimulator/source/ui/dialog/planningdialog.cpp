#include "ui/dialog/planningdialog.h"

#include "ui_planningdialog.h"

#include <QtWidgets/QTableWidget>
#include <QtWidgets/QDialog>

#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG

using std::vector;

namespace GComponent {
/*________________________________________CONSTRUCTOR_DESTRUCTOR_________________________________________*/
PlanningDialog::PlanningDialog(QWidget* parent):
	QDialog(parent),
	ui_ptr_(new Ui::PlanningDialog)
{
	ui_ptr_->setupUi(this);
	
}

PlanningDialog::~PlanningDialog()
{
	delete ui_ptr_;
}

void PlanningDialog::AppendPlannableObject(const std::string& obj)
{
	ui_ptr_->ptp_obj_combo   ->addItem(obj.data());
	ui_ptr_->line_obj_combo  ->addItem(obj.data());
	ui_ptr_->spline_obj_combo->addItem(obj.data());
	ui_ptr_->circle_obj_combo->addItem(obj.data());
	ui_ptr_->left_obj_combo  ->addItem(obj.data());
	ui_ptr_->right_obj_combo ->addItem(obj.data());
}

void PlanningDialog::RemovePlannableObject(const std::string& obj)
{	
	ui_ptr_->ptp_obj_combo   ->removeItem(ui_ptr_->ptp_obj_combo   ->findText(obj.data()));
	ui_ptr_->line_obj_combo  ->removeItem(ui_ptr_->line_obj_combo  ->findText(obj.data()));
	ui_ptr_->circle_obj_combo->removeItem(ui_ptr_->circle_obj_combo->findText(obj.data()));
	ui_ptr_->spline_obj_combo->removeItem(ui_ptr_->spline_obj_combo->findText(obj.data()));
	ui_ptr_->left_obj_combo  ->removeItem(ui_ptr_->left_obj_combo  ->findText(obj.data()));
	ui_ptr_->right_obj_combo ->removeItem(ui_ptr_->right_obj_combo ->findText(obj.data()));
}

/*_________________________________________PROTECTED METHODS_______________________________________________*/
	

/*_________________________________________PRIVATE METHODS_______________________________________________*/
	void PlanningDialog::PTPMotionExecution()
	{
		enum SpaceType: int{
			Joint = 0, Descarte
		}target_space = static_cast<SpaceType>(ui_ptr_->ptp_target_combo->currentIndex());

		QString obj_name = ui_ptr_->ptp_obj_combo->currentText();
		float   max_vel  = ui_ptr_->ptp_vel_val->text().toFloat();
		float   max_acc  = ui_ptr_->ptp_acc_val->text().toFloat();
		
		switch (target_space)
		{
		case Joint:		PTPJMotionExecution(obj_name, max_vel, max_acc); break;		
		case Descarte:  PTPCMotionExecution(obj_name, max_vel, max_acc); break;
		}
	}

	void PlanningDialog::PTPJMotionExecution(const QString& obj_name, float max_vel, float max_acc)
	{
		vector<float> targets = GetOneRowFromTable(ui_ptr_->jspace_table, 0);	
		emit RequestPTPMotionJSpace(obj_name, max_vel, max_acc, targets);
	}

	void PlanningDialog::PTPCMotionExecution(const QString& obj_name, float max_vel, float max_acc)
	{
		vector<float> targets = GetOneRowFromTable(ui_ptr_->cspace_table, 0);	
		emit RequestPTPMotionCSpace(obj_name, max_vel, max_acc, targets);
	}

	void PlanningDialog::LineMotionExecution()
	{
		QString		  obj_name	  = ui_ptr_->line_obj_combo->currentText();
		float		  max_vel	  = ui_ptr_->line_vel_val->text().toFloat();
		float		  max_acc	  = ui_ptr_->line_acc_val->text().toFloat();
		float		  max_ang_vel = ui_ptr_->line_ang_vel_val->text().toFloat();
		float		  max_ang_acc = ui_ptr_->line_ang_acc_val->text().toFloat();
		vector<float> targets	  = GetOneRowFromTable(ui_ptr_->line_cspace_table, 0);

		emit RequestLineMotion(obj_name, max_vel, max_acc, max_ang_vel, max_ang_acc, targets);
	}

	void PlanningDialog::CircleMotionExecution()
	{
		QString		  obj_name    = ui_ptr_->circle_obj_combo->currentText();
		float		  max_vel     = ui_ptr_->circle_vel_val->text().toFloat();
		float		  max_acc     = ui_ptr_->circle_acc_val->text().toFloat();
		float		  max_ang_vel = ui_ptr_->cir_ang_vel_val->text().toFloat();
		float		  max_ang_acc = ui_ptr_->cir_ang_acc_val->text().toFloat();
		vector<float> targets	  = GetOneRowFromTable(ui_ptr_->cir_cspace_table, 0);
		vector<float> waypoint(3);
		waypoint[0] = ui_ptr_->cir_pos_x_val->text().toFloat();
		waypoint[1] = ui_ptr_->cir_pos_y_val->text().toFloat();
		waypoint[2] = ui_ptr_->cir_pos_z_val->text().toFloat();
		
		emit RequestCircleMotion(obj_name, max_vel, max_acc, max_ang_vel, max_ang_acc, targets, waypoint);
	}

	void PlanningDialog::SplineMotionExecution()
	{
		QString				  obj_name    = ui_ptr_->spline_obj_combo->currentText();
		QString				  spline_name = ui_ptr_->spline_type_combo->currentText();
		float				  max_vel     = ui_ptr_->spline_vel_val->text().toFloat();
		float				  max_acc     = ui_ptr_->spline_acc_val->text().toFloat();
		float				  max_ang_vel = ui_ptr_->spline_ang_vel_val->text().toFloat();
		float				  max_ang_acc = ui_ptr_->spline_ang_acc_val->text().toFloat();
		vector<float>		  targets     = GetOneRowFromTable(ui_ptr_->spline_cspace_table, 0);
		vector<vector<float>> waypoints   = GetDatasFromTable(ui_ptr_->spline_waypoints_table);

		emit RequestSplineMotion(obj_name, spline_name, max_vel, max_acc, max_ang_vel, max_ang_acc, targets, waypoints);
	}

	void PlanningDialog::KeeperMotionExecution()
	{
		QString		  obj_name = ui_ptr_->keeper_obj_combo->currentText();
		float		  time     = ui_ptr_->keeper_time_val->text().toFloat();
		vector<float> targets  = GetOneRowFromTable(ui_ptr_->line_cspace_table, 0);

		emit RequestKeeperMotion(obj_name, time, targets);
	}

	void PlanningDialog::DualMotionExecution()
	{
		enum DualType : int {
			Sync = 0, Async
		} type = static_cast<DualType>(ui_ptr_->dual_mode_combo->currentIndex());
		
		switch (type) {
		case Sync:	DualSyncMotionExecution();  break;
		case Async: DualAsyncMotionExecution(); break;
		}
	}

	void PlanningDialog::DualSyncMotionExecution()
	{
		enum MotionType : int {
			Line = 0, Circle
		} type = static_cast<MotionType>(ui_ptr_->sync_type_combo->currentIndex());
		
		vector<QString>		  objs		= { ui_ptr_->left_obj_combo->currentText(), ui_ptr_->right_obj_combo->currentText()};
		vector<float>		  max_vels	= { ui_ptr_->sync_vel_val_left->text().toFloat(),ui_ptr_->sync_vel_val_right->text().toFloat() };
		vector<float>		  max_accs	= { ui_ptr_->sync_acc_val_left->text().toFloat(),ui_ptr_->sync_acc_val_right->text().toFloat() };
		vector<vector<float>> bias		= { GetOneRowFromTable(ui_ptr_->sync_bias_tbl_left, 0), GetOneRowFromTable(ui_ptr_->sync_bias_tbl_right, 0)};
		vector<float>		  target	= GetOneRowFromTable(ui_ptr_->sync_target_tbl, 0);

		switch (type) {				   
		case Line:	 
			emit RequestDualSyncLineMotion(objs, max_vels, max_accs, target, bias);
			break;
		case Circle: {
			vector<float> waypoint(3);
			waypoint[0] = ui_ptr_->dual_sync_waypoint_x->text().toFloat();
			waypoint[1] = ui_ptr_->dual_sync_waypoint_y->text().toFloat();
			waypoint[2] = ui_ptr_->dual_sync_waypoint_z->text().toFloat();
			emit RequestDualSyncCircleMotion(objs, max_vels, max_accs, target, bias, waypoint);
			break;
		}
		}
	}

	void PlanningDialog::DualAsyncMotionExecution()
	{
		//TODO: complete this methods
		std::printf("DualAsyncMotionExecution No Implementation\n");		
	}

	vector<float> PlanningDialog::GetOneColFromTable(QTableWidget* tbl, int cow_idx)
	{
		if (!tbl) return vector<float>{};
		vector<float> vals(tbl->rowCount());
		for (int i = 0; i < vals.size(); ++i) {
			vals[i] = tbl->item(i, cow_idx)->text().toFloat();
		}
		return vals;
	}

	std::vector<float> PlanningDialog::GetOneRowFromTable(QTableWidget* tbl, int row_idx)
	{
		if (!tbl) return vector<float>{};
		vector<float> vals(tbl->columnCount());
		for (int i = 0; i < vals.size(); ++i) {
			vals[i] = tbl->item(row_idx, i)->text().toFloat();
		}
		return vals;
	}

	vector<vector<float>> PlanningDialog::GetDatasFromTable(QTableWidget* tbl)
	{
		vector<vector<float>> vals(tbl->rowCount(), vector<float>(tbl->columnCount(), 0.0f));
		for (int i = 0; i < tbl->rowCount(); ++i) {
			for (int j = 0; j < tbl->columnCount(); ++j) {
				QTableWidgetItem* item = tbl->item(i, j);
				if (item) {
					vals[i][j] = item->text().toFloat();
				}
			}
		}
		return vals;
	}

	std::vector<QString> PlanningDialog::GetCurrentObjName() const
	{
		std::vector<QString> obj_names;
		for (auto& child : ui_ptr_->planning_stacked->currentWidget()->children()) {
			if (child->objectName().contains("obj_combo")) {
				obj_names.push_back(dynamic_cast<QComboBox*>(child)->currentText());
			}
		}
		return obj_names;
	}

/*________________________________________PRIVATE SLOTS__________________________________________________*/
	void PlanningDialog::on_execution_button_clicked()
	{
		enum MotionType : int{
			NONE = 0, PTP, Line, Circle, Spline, Keeper, Dual
		}motion_type = static_cast<MotionType>(ui_ptr_->type_combo->currentIndex());

		switch (motion_type)
		{
		case PTP:	 PTPMotionExecution();	  break;
		case Line:	 LineMotionExecution();	  break;
		case Circle: CircleMotionExecution(); break;
		case Spline: SplineMotionExecution(); break;
		case Keeper: KeeperMotionExecution(); break;
		case Dual:   DualMotionExecution();	  break;
		default:	 break;
		}
		
	}

	void PlanningDialog::on_cancel_button_clicked()
	{		
		close();
	}

	void PlanningDialog::on_stop_button_clicked()
	{
		constexpr const int kStopStatus = 0;
		std::vector<QString> obj_names = GetCurrentObjName();		
		if (obj_names.empty()) return;
		emit RequestChangeCurrentTaskStatus(obj_names, kStopStatus);
	}

	void PlanningDialog::on_pause_button_clicked()
	{
		constexpr const int kPauseStatus = 1;
		std::vector<QString> obj_names = GetCurrentObjName();
		if (obj_names.empty()) return;
		emit RequestChangeCurrentTaskStatus(obj_names, kPauseStatus);
	}

	void PlanningDialog::on_resume_button_clicked()
	{
		constexpr const int kResumeStatus = 2;
		std::vector<QString> obj_names = GetCurrentObjName();
		if (obj_names.empty()) return;
		emit RequestChangeCurrentTaskStatus(obj_names, kResumeStatus);
	}

	void PlanningDialog::on_spline_waypoints_add_button_clicked()
	{
		QTableWidget& table = *ui_ptr_->spline_waypoints_table;
		table.insertRow(table.rowCount());
		for (int i = 0; i < table.columnCount(); ++i) {
			table.setItem(table.rowCount() - 1, i, new QTableWidgetItem(QString("0.0")));
		}
	}

	void PlanningDialog::on_spline_waypoints_del_button_clicked()
	{
		QTableWidget& table = *ui_ptr_->spline_waypoints_table;
		auto range = table.selectedRanges();
		if (range.empty()) return;
		int min_row = INT_MAX, max_row = INT_MIN;
		for (auto& sel: range) {
			min_row = std::min(sel.topRow(), min_row);
			max_row = std::max(sel.bottomRow(), max_row);
		}
		
		for (int row = max_row; row >= min_row; --row) {
			table.removeRow(row);			
		}		
	}

	void PlanningDialog::on_spline_waypoints_clear_button_clicked()
	{
		QTableWidget& table = *ui_ptr_->spline_waypoints_table;
		for (int row = table.rowCount() - 1; row >= 0; --row) {
			table.removeRow(row);
		}
	}

	void PlanningDialog::on_sync_type_combo_activated(int idx)
	{
		bool enable = idx;
		ui_ptr_->dual_circle_waypoint_text->setEnabled(enable);
		ui_ptr_->dual_sync_waypoint_x->setEnabled(enable);
		ui_ptr_->dual_sync_waypoint_x_label->setEnabled(enable);
		ui_ptr_->dual_sync_waypoint_y->setEnabled(enable);
		ui_ptr_->dual_sync_waypoint_y_label->setEnabled(enable);
		ui_ptr_->dual_sync_waypoint_z->setEnabled(enable);
		ui_ptr_->dual_sync_waypoint_z_label->setEnabled(enable);
	}

	void PlanningDialog::on_target_combo_currentIndexChanged(int idx)
	{
		emit GetTargetOptimizer(idx);
	}

	void PlanningDialog::on_selfmotion_combo_currentIndexChanged(int idx)
	{
		emit GetSelfMotionOptimizer(idx);
	}
}