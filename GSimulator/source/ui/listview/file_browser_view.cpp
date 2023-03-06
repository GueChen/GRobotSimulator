#include "ui/listview/file_browser_view.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QAbstractFileIconProvider>
#include <QtGui/QDrag>

#ifdef _DEBUG
#include <QDebug>
#include <iostream>
#include <QtCore/QMimeData>
#endif // DEBUG

GComponent::FileBrowserView::FileBrowserView(QWidget* parent, const QString& dft_dir):
	QListView(parent),
	model_ptr_(std::make_unique<QFileSystemModel>())
{
	// configure  model setting
	model_ptr_->setRootPath(QDir::currentPath() + dft_dir);
	
	setModel(model_ptr_.get());
	root_index_ = model_ptr_->index(model_ptr_->rootPath());
	setRootIndex(root_index_);

	setViewMode  (QListView::IconMode);
	setMovement  (QListView::Static);
	setResizeMode(QListView::Adjust);
	setLayoutMode(QListView::Batched);

	setUniformItemSizes(true);

	connect(this, &FileBrowserView::doubleClicked, this, &FileBrowserView::DoubleClickedSlot);

	setDragEnabled(true);

	setGridSize(QSize(kIconInitSize, kIconInitSize));
	setIconSize(QSize(kIconInitSize - kHorizontalSizeDiff, 
					  kIconInitSize - kVerticalSizeDiff));
}

GComponent::FileBrowserView::~FileBrowserView()
{}

void GComponent::FileBrowserView::setRootIndex(const QModelIndex & index)
{
	QListView::setRootIndex(index);
	QDir dir(model_ptr_->filePath(root_index_.parent()));
	emit RootDirChange(dir.relativeFilePath(model_ptr_->filePath(index)));
}

void GComponent::FileBrowserView::mousePressEvent(QMouseEvent* event)
{
	QListView::mousePressEvent(event);
	QModelIndex index = indexAt(event->position().toPoint());
	if (index.isValid()) {
		QFileInfo info = model_ptr_->fileInfo(index);
		QIcon     icon = model_ptr_->iconProvider()->icon(info);
		
		QMimeData* mime = model_ptr_->mimeData({ index });
		QDrag*     drag = new QDrag(this);
		drag->setMimeData(mime);
		drag->setPixmap(icon.pixmap(iconSize()));
		drag->setHotSpot(event->position().toPoint());

		//Qt::DropAction drop_aciton = drag->exec(Qt::CopyAction);
	}
}

void GComponent::FileBrowserView::wheelEvent(QWheelEvent* event)
{
	QListView::wheelEvent(event);
	int delta = event->angleDelta().y() / 120;
	QSize new_grid_size = gridSize() + QSize(delta, delta);
	if (kMinIconSize <= new_grid_size.width() && new_grid_size.width() <= kMaxIconSize) {
		setGridSize(new_grid_size);
		setIconSize(iconSize() + QSize(delta, delta));
	}
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


