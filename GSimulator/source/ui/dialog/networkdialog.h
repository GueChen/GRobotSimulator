/**
 *  @file  	networkdialog.h
 *  @brief 	network dialog responsible for network link with robots.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Sep 1st, 2022
 **/
#ifndef __NETWORK_DIALOG_H
#define __NETWORK_DIALOG_H

#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
	class NetworkDialog;
}
QT_END_NAMESPACE

namespace GComponent {

class NetworkDialog : public QDialog {

 Q_OBJECT

public:
	explicit NetworkDialog(QWidget* parent = nullptr);
	~NetworkDialog();

	void AppendBinableObject (const std::string& obj);
	void RemoveBinnableObject(const std::string& obj);

signals:
	void RequestLinkClientToServer(const QString& obj_name, 
								   QString		  ip_address, 
								   QString		  port);
	void RequestAsyncReceiver	  (const QString& obj_name, 
								   bool			  flag);
	void RequestQuit			  (const QString& obj_name);
	void RequestHigherAurthority  (const QString& obj_name);
	void RequestModeChange		  (const QString& obj_name,
								   int			  mode);

public slots:	
	void LinkStateChange		  (QString		  obj_name, 
								   bool			  flag);
	void RankLevelChange		  (QString		  obj_name, 
								   int			  rank);
	void ResponseAsyncStatus	  (QString		  obj_name, 
								   bool			  flag);
	void ResponseLinkError		  ();
	

private slots:	
	void on_left_connect_button_clicked	();
	void on_left_async_button_clicked	();
	void on_left_quit_button_clicked	();
	void on_left_request_button_clicked	();
	void on_left_mode_button_clicked	();

	void on_right_connect_button_clicked();
	void on_right_async_button_clicked	();
	void on_right_quit_button_clicked	();
	void on_right_request_button_clicked();
	void on_right_mode_button_clicked	();


private:
	Ui::NetworkDialog*		ui_ptr_		= nullptr;
};

} // !namespace GComoponent


#endif // !_NETWORK_DIALOG_H

