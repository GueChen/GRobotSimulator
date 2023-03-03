
#include "widgetbuilder.h"

#include "pattern/widgetbuilder.h"

namespace GComponent{

QWidgetBuilder& QWidgetBuilder::AddLabel(const QString& text, const QString& obj_name, const QString& qss, const QSize& mini_size, const QSize& max_size, Qt::Alignment align)
{
	QLabel* label = new QLabel(text, widget_paradigm_);
	label->setObjectName(obj_name);
	label->setStyleSheet(qss);
	label->setMinimumSize(mini_size);
	label->setMaximumSize(max_size);
	label->setAlignment(align);
	widget_paradigm_->layout()->addWidget(label);
	return *this;	
}

QWidgetBuilder& QWidgetBuilder::AddLineEdit(const QString& text, const QString& obj_name, const QString& qss, const QSize& mini_size, const QSize& max_size, Qt::Alignment align)
{
	QLineEdit * edit = new QLineEdit(text, widget_paradigm_);
	edit->setObjectName(obj_name);
	edit->setStyleSheet(qss);
	edit->setMinimumSize(mini_size);
	edit->setMaximumSize(max_size);
	edit->setAlignment(align);
	widget_paradigm_->layout()->addWidget(edit);
	return *this;
}

}
