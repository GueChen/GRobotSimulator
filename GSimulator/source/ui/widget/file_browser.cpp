#include "ui/widget/file_browser.h"

#include "ui_file_browser_widget.h"

#include <iostream>

constexpr const int kIconInitSize	    = 75;
constexpr const int kHorizontalSizeDiff = 25;
constexpr const int kVerticalSizeDiff   = 30;

GComponent::FileBrowser::FileBrowser(QWidget* parent):
	QWidget(parent),
	ui_(new Ui::FileBrowser)
{
	ui_->setupUi(this);

	connect(ui_->size_adjust_slider, &QSlider::valueChanged, 
		[&view = ui_->content_view, &text = ui_->icon_size](int size){
		view->setGridSize(QSize(size, size));
		view->setIconSize(QSize(size - kHorizontalSizeDiff, size - kVerticalSizeDiff));
		text->setText(QString::number(size));
	});

	connect(ui_->back_button, &QToolButton::clicked, 
		[&view = ui_->content_view](bool flag) {
			view->BackToParentDir();
	});
	
	connect(ui_->content_view, &FileBrowserView::RootDirChange, 
		[&text = ui_->relative_dir](const QString& relative_dir){
		text->setText(relative_dir);
	});

	ui_->size_adjust_slider->setValue(kIconInitSize);
}

GComponent::FileBrowser::~FileBrowser()
{
	delete ui_;
}
