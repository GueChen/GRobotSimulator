#include "extensiblelist.h"

#include "pattern/widgetbuilder.h"

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG

namespace GComponent{
ExtensibleList::ExtensibleList(QWidget* parent):
	QWidget(parent)	
{
		
}

void ExtensibleList::SetProtoTypeName(const QString& proto_name)
{	
	if (prototype_)
	{
		prototype_->setObjectName(proto_name);
	}
}

bool ExtensibleList::AddItem()
{
	//if (!prototype_) return false;		
	QWidgetBuilder builder(new QHBoxLayout);
	const QString  count = QString::number(items_.size());
	const QSize	   label_min_size(20, 0);
	const QSize	   line_edit_min_size(50, 0);
	const QSize	   default_max_size(16777215, 16777215);
	Qt::Alignment  align = Qt::AlignCenter;

	builder.AddLabel("Joint" + count,	
								QString("Joint")		+ count, label_head.data(),		line_edit_min_size,	default_max_size, align).
			AddLabel("a",		QString("a_label_")		+ count, label_a.data(),		label_min_size,		default_max_size, align).
			AddLineEdit("0.0",  QString("a_val_")		+ count, line_edit_qss.data(),	line_edit_min_size, default_max_size, align).
			AddLabel("alp",		QString("alpha_label_")	+ count, label_alpha.data(),	label_min_size,		default_max_size, align).
			AddLineEdit("0.0",  QString("alpha_val_")	+ count, line_edit_qss.data(),	line_edit_min_size, default_max_size, align).
			AddLabel("d",		QString("d_label_")		+ count, label_d.data(),		label_min_size,		default_max_size, align).
			AddLineEdit("0.0",	QString("d_val_")		+ count, line_edit_qss.data(),	line_edit_min_size, default_max_size, align).
			AddLabel("xi",		QString("xita_label_")	+ count, label_xita.data(),		label_min_size,		default_max_size, align).
			AddLineEdit("0.0",  QString("xita_val_")	+ count, line_edit_qss.data(),	line_edit_min_size, default_max_size, align);
	QWidget* widget = builder.GetWidget();
	widget->setStyleSheet(widget_qss.data());
	items_.emplace_back(builder.GetWidget());
	layout()->addWidget(items_.back());	
	return true;
}

bool ExtensibleList::RemoveItem()
{
	if(items_.empty()) return false;
		
	QWidget* delete_one = items_.back();
	items_.pop_back();

	if (delete_one) {				
		layout()->removeWidget(delete_one);
		delete delete_one;
		delete_one = nullptr;
	}
			
	return true;
}

}
