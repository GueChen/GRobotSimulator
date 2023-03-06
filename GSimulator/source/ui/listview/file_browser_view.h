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
	friend class FileBrowser;
public:
	FileBrowserView(QWidget* parent = nullptr, const QString& dft_dir = "/asset");
	~FileBrowserView();

	void setRootIndex(const QModelIndex& index) override;

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event)	     override;
signals:
	void RootDirChange(const QString& dir);

public slots:
	void BackToParentDir();

private slots:
	void DoubleClickedSlot(const QModelIndex& index);

private:
	std::unique_ptr<QFileSystemModel> model_ptr_;
	QModelIndex						  root_index_;

private:
	static constexpr const int kIconInitSize	   = 75;
	static constexpr const int kMaxIconSize        = 100;
	static constexpr const int kMinIconSize        = 50;
	static constexpr const int kHorizontalSizeDiff = 25;
	static constexpr const int kVerticalSizeDiff   = 30;
};

}	// !namespace GComponent

#endif // !__FILE_BROWSER_H

