#include "ui/widget/file_browser.h"

#include "ui_file_browser_widget.h"

#include <iostream>

GComponent::FileBrowser::FileBrowser(QWidget* parent):
	QWidget(parent),
	ui_(new Ui::FileBrowser)
{
	ui_->setupUi(this);
	
	connect(ui_->size_adjust_slider, &QSlider::valueChanged, 
		[&view = ui_->content_view, &text = ui_->icon_size](int size){
		view->setGridSize(QSize(size, size));
		view->setIconSize(QSize(size - FileBrowserView::kHorizontalSizeDiff, 
								size - FileBrowserView::kVerticalSizeDiff));
		text->setText(QString::number(size));		
	});
	connect(ui_->content_view, &FileBrowserView::iconSizeChanged, 
		[&slider = ui_->size_adjust_slider, &text = ui_->icon_size](const QSize& size) {
		int input = size.width() + FileBrowserView::kHorizontalSizeDiff;
		slider->setValue(input);
		text->setText(QString::number(input));
	});

	connect(ui_->back_button, &QToolButton::clicked, 
		[&view = ui_->content_view](bool flag) {
			view->BackToParentDir();
	});
	
	connect(ui_->content_view, &FileBrowserView::RootDirChange, 
		[&text = ui_->relative_dir](const QString& relative_dir){
		text->setText(relative_dir);
	});

	ui_->size_adjust_slider->setMaximum(FileBrowserView::kMaxIconSize);
	ui_->size_adjust_slider->setMinimum(FileBrowserView::kMinIconSize);
	ui_->size_adjust_slider->setValue(FileBrowserView::kIconInitSize);
}

GComponent::FileBrowser::~FileBrowser()
{
	delete ui_;
}
