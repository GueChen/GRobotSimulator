#include "convex_decomposition_dialog.h"

#include "ui_convex_decomposition_dialog.h"

namespace GComponent{

ConvexDecompositionDialog::ConvexDecompositionDialog(QWidget* parent):
	QDialog(parent),
	ui_ptr_(new Ui::ConvDecoDialog)
{
	ui_ptr_->setupUi(this);
	ui_ptr_->scale->SetRange(0.0f, std::numeric_limits<float>::max());
	ui_ptr_->scale->SetStep(0.01f);
}

ConvexDecompositionDialog::~ConvexDecompositionDialog()
{
	delete ui_ptr_;
}

void ConvexDecompositionDialog::showEvent(QShowEvent* event)
{
	ui_ptr_->conv_count->setText(QString::number(10));
	ui_ptr_->max_vert  ->setText(QString::number(32));
	ui_ptr_->scale	   ->SetValue(QVector3D(1.0f, 1.0f, 1.0f));
	ui_ptr_->show_mesh ->setChecked(false);
	ui_ptr_->need_log  ->setChecked(false);
	
	QDialog::showEvent(event);
}

void ConvexDecompositionDialog::on_ok_button_clicked()
{
	union Flags
	{
		bool booleans[2];
		int  val;
	} flags; flags.val = 0;
	uint32_t conv_count = ui_ptr_->conv_count->text().toUInt(&flags.booleans[0]);
	uint32_t max_vert   = ui_ptr_->max_vert->text().toUInt(&flags.booleans[1]);	
	
	if (flags.val == 0) {
		return;
	}

	QVector3D scale_ori = ui_ptr_->scale->GetValue();
	float    scale[3];
	scale[0] = scale_ori[0]; scale[1] = scale_ori[1]; scale[2] = scale_ori[2];

	bool show_mesh = ui_ptr_->show_mesh->isChecked();
	bool need_log  = ui_ptr_->show_mesh->isChecked();

	emit RequestDecomposition(conv_count, max_vert, scale, show_mesh, need_log);
}



}