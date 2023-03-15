/**
 *  @file  	shader_creator_dialog.h
 *  @brief 	This dialog used to generate new shader in resource systems.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 15th, 2023
 **/
#ifndef __SHADER_CREATOR_DIALOG
#define __SHADER_CREATOR_DIALOG

#include <QtWidgets/QDialog>

#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class ShaderCreatorDialog; }
QT_END_NAMESPACE

namespace GComponent {

class ShaderCreatorDialog : public QDialog {
	struct ShaderPath{
		QString vert_path;
		QString frag_path;
		QString geom_path;
	};

	Q_OBJECT
public:
	explicit ShaderCreatorDialog(QWidget* parent = nullptr);
	~ShaderCreatorDialog();

signals:
	void ShaderCreateRequest(const QString& name, const QString& vert, const QString& frag, const QString& geom);

private:
	std::unique_ptr<Ui::ShaderCreatorDialog> ui_;	
	ShaderPath								 path_;

};

}


#endif // !__SHADER_CREATOR_DIALOG
