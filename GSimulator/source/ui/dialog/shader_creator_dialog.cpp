#include "ui/dialog/shader_creator_dialog.h"

#include "ui_shader_creator_dialog.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtGui/QMouseEvent>

#include <QDebug>

namespace GComponent {
ShaderCreatorDialog::ShaderCreatorDialog(QWidget* parent):
	QDialog(parent),
	ui_(new Ui::ShaderCreatorDialog)
{
	ui_->setupUi(this);
	ui_->vertex_shader  ->setReadOnly(true);
	ui_->fragment_shader->setReadOnly(true);
	ui_->geometry_shader->setReadOnly(true);

	ui_->vertex_shader  ->setPlaceholderText(QString("click select .vert"));
	ui_->fragment_shader->setPlaceholderText(QString("click select .frag"));
	ui_->geometry_shader->setPlaceholderText(QString("click select .geom"));
	
	ui_->vertex_shader  ->setAlignment(Qt::AlignHCenter);
	ui_->fragment_shader->setAlignment(Qt::AlignHCenter);
	ui_->geometry_shader->setAlignment(Qt::AlignHCenter);

	ui_->list_box->hide();

	connect(ui_->vertex_shader,   &ClickedEdit::Clicked, [this]() {
		QString file_path = QFileDialog::getOpenFileName(this, "Select Vert File", "./Shader/Vertex", "Text files(*.vert)");
		QFileInfo file(file_path);
		ui_->vertex_shader->setText(file.baseName());			
		path_.vert_path = file.absoluteFilePath();
	});
	connect(ui_->fragment_shader, &ClickedEdit::Clicked, [this]() {
		QString file_path = QFileDialog::getOpenFileName(this, "Select Frag File", "./Shader/Fragment", "Text files(*.frag)");
		QFileInfo file(file_path);
		ui_->fragment_shader->setText(file.baseName());
		path_.frag_path = file.absoluteFilePath();
	});
	connect(ui_->geometry_shader, &ClickedEdit::Clicked, [this]() {
		QString file_path = QFileDialog::getOpenFileName(this, "Select Geom File", "./Shader/Geom", "Text files(*.geom)");
		QFileInfo file(file_path);
		ui_->geometry_shader->setText(file.baseName());
		path_.geom_path = file.absoluteFilePath();
	});

	connect(ui_->generate_button, &QPushButton::clicked, [this]() {
		QString name = ui_->shader_name->text();
		if (name.isEmpty() || path_.vert_path.isEmpty() || path_.frag_path.isEmpty()) {
			QMessageBox::critical(this, "Generate Error", "name, vert and frag can't be null", QMessageBox::Ok);
			return;
		}
		emit ShaderCreateRequest(ui_->shader_name->text(), path_.vert_path, path_.frag_path, path_.geom_path);
	});

	connect(this, &ShaderCreatorDialog::finished, [this](int) { 
		path_ = {};
		ui_->vertex_shader  ->clear();
		ui_->fragment_shader->clear();
		ui_->geometry_shader->clear();
	});
	
}

ShaderCreatorDialog::~ShaderCreatorDialog() = default;


}