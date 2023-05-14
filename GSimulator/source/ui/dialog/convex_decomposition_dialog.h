/**
 *  @file  	convex_decomposition_dialog.h
 *  @brief 	This file supply a vhacd ui option for user.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 14th, 2023
 **/
#ifndef __CONVEX_DECOMPOSITION_DIALOG_H
#define __CONVEX_DECOMPOSITION_DIALOG_H

#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
	class ConvDecoDialog;
}
QT_END_NAMESPACE

namespace GComponent {

class ConvexDecompositionDialog : public QDialog {
	Q_OBJECT
public:
	explicit ConvexDecompositionDialog(QWidget* parent = nullptr);
	~ConvexDecompositionDialog();

protected:
	void showEvent(QShowEvent* evnet) override;

protected slots:
	void on_ok_button_clicked();

signals:
	void RequestDecomposition(uint32_t conv_count, 
							  uint32_t max_vert_count, 
							  float	   scale[3], 
							  bool	   show_mesh,
							  bool     need_log); 

private:
	Ui::ConvDecoDialog* ui_ptr_ = nullptr;
};

} // !namespace GComponent

#endif // !__CONVEX_DECOMPOSITION_DIALOG_H
