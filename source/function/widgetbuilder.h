/**
 *  @file  	widgetbuilder.h
 *  @brief 	This class is a helper class to build a widget.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 25, 2022
 **/
#ifndef _WIDGET_BUILDER_H
#define _WIDGET_BUILDER_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtCore/QString>

namespace GComponent {
class QWidgetBuilder {
public:
	QWidgetBuilder(QLayout* layout = new QVBoxLayout) { 
		widget_paradigm_ = new QWidget; 
		layout->setParent(widget_paradigm_);
		widget_paradigm_->setLayout(layout); 
	}
	~QWidgetBuilder() { 
		if (!is_returned_) delete widget_paradigm_; 
	};
	
	[[nodiscard]] inline 
	QWidget* 
	GetWidget(){
		is_returned_ = true; 
		return widget_paradigm_;
	}

	QWidgetBuilder& AddLabel(const QString& text,	 const QString& obj_name = "") {
		QLabel *  label = new QLabel(text, widget_paradigm_);
		label->setObjectName(obj_name);
		widget_paradigm_->layout()->addWidget(label);
		return *this;
	}
	QWidgetBuilder& AddLineEdit(const QString& text, const QString& obj_name = "") {
		QLineEdit* edit = new QLineEdit(text, widget_paradigm_);
		edit->setObjectName(obj_name);
		widget_paradigm_->layout()->addWidget(edit);
		return *this;
	}
	QWidgetBuilder& AddSpacer(QSizePolicy::Policy h_policy, QSizePolicy::Policy v_policy) {
		widget_paradigm_->layout()->addItem(new QSpacerItem(40, 20, h_policy, v_policy));
		return *this;
	}

private:
	bool      is_returned_		= false;
	QWidget * widget_paradigm_;
};
}

#endif // !_WIDGET_BUILDER_H
