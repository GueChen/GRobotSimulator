#include "ui/dialog/robotcreatedialog.h"

#include "ui_robotcreatedialog.h"

#include <QtWidgets/QLineEdit>

#include <vector>
#include <unordered_map>

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG

namespace GComponent {
RobotCreateDialog::RobotCreateDialog(QWidget* parent):
	QDialog(parent),
	ui_ptr_(new Ui::RobotCreateDialog)
{
	ui_ptr_->setupUi(this);		
}

RobotCreateDialog::~RobotCreateDialog()
{
	delete ui_ptr_;
}

/*__________________________PRIVATE METHODS___________________________________________________*/
void GComponent::RobotCreateDialog::ClearItem()
{
	while (robot_joint_num_ > 0) 
	{
		RemoveItem();
	}	
}

void GComponent::RobotCreateDialog::AddItem()
{
	if (ui_ptr_->extensible_list->AddItem()) 
	{
		++robot_joint_num_;
	}
}

void GComponent::RobotCreateDialog::RemoveItem()
{
	if (ui_ptr_->extensible_list->RemoveItem()) 
	{
		--robot_joint_num_;
	}
}

/*__________________________PRIVATE SLOTS METHODS_____________________________________________*/
void RobotCreateDialog::on_add_button_clicked()
{	
	AddItem();
}

void RobotCreateDialog::on_remove_button_clicked()
{
	RemoveItem();
}

void RobotCreateDialog::on_ok_button_clicked()
{
	auto line_edits = ui_ptr_->extensible_list->findChildren<QLineEdit*>();
	const int parameter_num = 4;
	const int joint_num = line_edits.size() / parameter_num;
	std::vector<std::vector<float>> parameters(joint_num, std::vector<float>(parameter_num, 0.0f));
	
	std::unordered_map<QString, int> pos_idx_dict{
		{QString("alpha"), 0},
		{QString("a"),	   1},
		{QString("xita"),  2},
		{QString("d"),	   3}
	};

	for (auto& line_edit : line_edits) {
		QString name	= line_edit->objectName();		
		int count		= name.mid(name.lastIndexOf('_') + 1).toInt();
		int param_idx	= pos_idx_dict[name.mid(0, name.indexOf('_'))];
		parameters[count][param_idx] = line_edit->text().toFloat();		
	}

#ifdef _DEBUG
	for (auto& param : parameters) {
		std::cout << "Joint |";
		for (auto& data : param) {
			std::cout << data << " ";
		}
		std::cout << std::endl;
	}
#endif // _DEBUG
	
	emit RobotCreateRequestMDH(parameters);

	ClearItem();	
}

void RobotCreateDialog::on_cancel_button_clicked()
{
	ClearItem();
}
}
