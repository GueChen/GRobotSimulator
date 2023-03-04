/**
 *  @file  	file_browser.h
 *  @brief  Using QListView combine with QFileSystemModel as a Icon display viewer, with some control override function.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 4th, 2023
 **/
#ifndef __FILE_BROWSER_H
#define __FILE_BROWSER_H

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class FileBrowser; }
QT_END_NAMESPACE

namespace GComponent {

class FileBrowser :public QWidget {
	Q_OBJECT
public:
	FileBrowser(QWidget* parent = nullptr);
	~FileBrowser();

private:
	Ui::FileBrowser* ui_;
};
}

#endif // !__FILE_BROWSER_H
