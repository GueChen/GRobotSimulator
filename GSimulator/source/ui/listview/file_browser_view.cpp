#include "ui/listview/file_browser_view.h"

#include <QDebug>

GComponent::FileBrowserView::FileBrowserView(QWidget* parent):
	QListView(parent),
	model_ptr_(std::make_unique<QFileSystemModel>())
{
	// configure  model setting
	model_ptr_->setRootPath(QDir::currentPath() + "/asset");
	
	setModel(model_ptr_.get());
	root_index_ = model_ptr_->index(model_ptr_->rootPath());
	setRootIndex(root_index_);

	setViewMode  (QListView::IconMode);
	setMovement  (QListView::Static);
	setResizeMode(QListView::Adjust);
	
	setUniformItemSizes(true);

	connect(this, &FileBrowserView::doubleClicked, this, &FileBrowserView::DoubleClickedSlot);
}

GComponent::FileBrowserView::~FileBrowserView()
{}

void GComponent::FileBrowserView::setRootIndex(const QModelIndex & index)
{
	QListView::setRootIndex(index);
	QDir dir(model_ptr_->filePath(root_index_.parent()));
	emit RootDirChange(dir.relativeFilePath(model_ptr_->filePath(index)));
}

void GComponent::FileBrowserView::BackToParentDir()
{
	if (rootIndex() != root_index_) {
		setRootIndex(rootIndex().parent());
	}
}

void GComponent::FileBrowserView::DoubleClickedSlot(const QModelIndex & index)
{
	if (model_ptr_->isDir(index)) {
		setRootIndex(index);
	}	
}


