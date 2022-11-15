#include "networkdialog.h"

#include "ui_networkdialog.h"

#include "base/global/global_qss.h"

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtCore/QMap>

#include <tuple>

// TODO: fit everything not only two obj
static const QString obj_table[] = {
	"left",
	"right"
};

namespace GComponent {

NetworkDialog::NetworkDialog(QWidget* parent) :
	QDialog(parent),
	ui_ptr_(new Ui::NetworkDialog)
{
	ui_ptr_->setupUi(this);
}

NetworkDialog::~NetworkDialog()
{
	delete ui_ptr_;
}

/*______________________________SLOTS METHODS__________________________________*/
/*______________________________PUBLIC METHODS_________________________________*/
void GComponent::NetworkDialog::LinkStateChange(QString obj_name, bool flag)
{	
	QLineEdit*   edit   = (obj_name == "left" ? ui_ptr_->left_link_state : ui_ptr_->right_link_state);
	QPushButton* button = (obj_name == "left" ? ui_ptr_->left_connect_button : ui_ptr_->right_connect_button);
	if (flag) {
		edit->setText("Connected");
		button->setText("break");
		button->setStyleSheet(red_button_qss.data());
	}
	else {
		edit->setText("None");
		button->setText("connect");
		button->setStyleSheet(normal_button_qss.data());
		RankLevelChange(obj_name, 0);
		ResponseAsyncStatus(obj_name, false);
	}		
}

void GComponent::NetworkDialog::RankLevelChange(QString obj_name, int rank)
{
	static QString ranks[4] = {
		"None",			// 0
		"Observer",		// 1
		"Requester",	// 2
		"Controller"	// 3
	};	
	
	QLineEdit*   edit   = (obj_name == "left" ? ui_ptr_->left_rank : ui_ptr_->right_rank);
	QPushButton* button = (obj_name == "left" ? ui_ptr_->left_request_button : ui_ptr_->right_request_button);
	edit->setText(ranks[rank]);	
	button->setEnabled(rank <= 1);
	
}

void NetworkDialog::ResponseLinkError()
{
	int ret = QMessageBox::critical(this, 
								   "Link Error", 
								   "Host may not exist\nplease check the hadware linkings status",
								   QMessageBox::Ok);
}

void NetworkDialog::ResponseAsyncStatus(QString obj_name, bool flag)
{
	QLineEdit* edit = (obj_name == "left" ? ui_ptr_->left_async_state : ui_ptr_->right_async_state);
	edit->setText(flag ? "On" : "Off");
}

/*______________________________PRIVATE METHODS________________________________*/
void NetworkDialog::on_left_connect_button_clicked()
{
	if(ui_ptr_->left_connect_button->text() == "connect"){
		emit RequestLinkClientToServer(obj_table[ui_ptr_->selected_tab->currentIndex()],
									   ui_ptr_->left_ip->text(), 
									   ui_ptr_->left_port->text());
	}
	else {
		emit RequestQuit(obj_table[ui_ptr_->selected_tab->currentIndex()]);
	}
}

void NetworkDialog::on_left_async_button_clicked()
{
	bool flag = ui_ptr_->left_async_state->text() == "Off";
	emit RequestAsyncReceiver(obj_table[ui_ptr_->selected_tab->currentIndex()], 
							  flag);
}

void NetworkDialog::on_left_request_button_clicked()
{
	emit RequestHigherAurthority(obj_table[ui_ptr_->selected_tab->currentIndex()]);
}

void NetworkDialog::on_left_mode_button_clicked()
{
	static QStringList modes = {"Normal", "V2R", "R2V"};
	static std::map<QString, int> idx_map = {
		{modes[0], 0}, {modes[1], 1}, {modes[2], 2}
	};	
	QLineEdit*	mode = ui_ptr_->left_mode;
	QString		ret	 = QInputDialog::getItem(this,
									"mode selection",
									"mode",
									modes,
									idx_map[mode->text()], false);
	mode->setText(ret);
	emit RequestModeChange(obj_table[ui_ptr_->selected_tab->currentIndex()], 
						   idx_map[ret]);
}

void NetworkDialog::on_left_quit_button_clicked()
{
	emit RequestQuit(obj_table[ui_ptr_->selected_tab->currentIndex()]);
}

void NetworkDialog::on_right_connect_button_clicked()
{
	if(ui_ptr_->right_connect_button->text() == "connect"){
		emit RequestLinkClientToServer(obj_table[ui_ptr_->selected_tab->currentIndex()],
									   ui_ptr_->right_ip->text(), 
									   ui_ptr_->right_port->text());
	}
	else {
		emit RequestQuit(obj_table[ui_ptr_->selected_tab->currentIndex()]);
	}
}

void NetworkDialog::on_right_async_button_clicked()
{
	bool flag = ui_ptr_->right_async_state->text() == "Off";
	emit RequestAsyncReceiver(obj_table[ui_ptr_->selected_tab->currentIndex()], 
							  flag);
}

void NetworkDialog::on_right_request_button_clicked()
{
	emit RequestHigherAurthority(obj_table[ui_ptr_->selected_tab->currentIndex()]);
}

void NetworkDialog::on_right_mode_button_clicked()
{
	static QStringList modes = {"Normal", "V2R", "R2V"};
	static std::map<QString, int> idx_map = {
		{modes[0], 0}, {modes[1], 1}, {modes[2], 2}
	};	
	QLineEdit*	mode = ui_ptr_->right_mode;
	QString		ret	 = QInputDialog::getItem(this,
									"mode selection",
									"mode",
									modes,
									idx_map[mode->text()], false);
	mode->setText(ret);
	emit RequestModeChange(obj_table[ui_ptr_->selected_tab->currentIndex()], 
						   idx_map[ret]);
}

void NetworkDialog::on_right_quit_button_clicked()
{
	emit RequestQuit(obj_table[ui_ptr_->selected_tab->currentIndex()]);
}

}