/**
 *  @file  	file_browser.h
 *  @brief 	Inheriented from list view combine with QFileSystemModel as a Icon display viewer.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 4th, 2023
 **/
#ifndef __FILE_BROWSER_VIEW_H
#define __FILE_BROWSER_VIEW_H

#include <QtWidgets/QListView>
#include <QtGui/QFileSystemModel>

#include <memory>

namespace GComponent {

class FileBrowserView : public QListView {
	Q_OBJECT
public:
	FileBrowserView(QWidget* parent = nullptr);
	~FileBrowserView();

	void setRootIndex(const QModelIndex& index) override;

signals:
	void RootDirChange(const QString& dir);

public slots:
	void BackToParentDir();

private slots:
	void DoubleClickedSlot(const QModelIndex& index);

private:
	std::unique_ptr<QFileSystemModel> model_ptr_;
	QModelIndex						  root_index_;
};

}	// !namespace GComponent

#endif // !__FILE_BROWSER_H

