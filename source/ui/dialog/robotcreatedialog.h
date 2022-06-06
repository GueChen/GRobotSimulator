#ifndef _ROBOT_CREATE_DIALOG_H
#define _ROBOT_CREATE_DIALOG_H

#include <QtWidgets/QDialog>

#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class RobotCreateDialog;
}
QT_END_NAMESPACE

namespace GComponent{

using std::vector;

class RobotCreateDialog : public QDialog
{
	Q_OBJECT
public:
	explicit RobotCreateDialog(QWidget* parent = nullptr);
	~RobotCreateDialog();

private:
	void AddItem();
	void RemoveItem();
	void ClearItem();

signals:
	void RobotCreateRequestSDH();
	void RobotCreateRequestMDH(const vector<vector<float>>&);
	void RobotCreateRequestTwist();
	
private slots:	
	void on_add_button_clicked();
	void on_remove_button_clicked();
	void on_ok_button_clicked();
	void on_cancel_button_clicked();

private:
	Ui::RobotCreateDialog* ui_ptr_			= nullptr;
	int					   robot_joint_num_ = 0;
};
	
}
#endif // !_ROBOT_CREATE_DIALOG_H

