/**
 *  @file  	widgetbuilder.h
 *  @brief 	This class is a helper class to build a widget.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 25, 2022
 **/
#ifndef _WIDGET_BUILDER_H
#define _WIDGET_BUILDER_H

#include "render/shader_property.hpp"

#include <QtWidgets/QWidget>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtCore/QString>

#include <unordered_map>
#include <string>

namespace GComponent {
enum class LayoutType {
	Vertical   = 0,
	Horizontal = 1,
	Grid	   = 2
};

class QWidgetBuilder {
public:
	QWidgetBuilder(QLayout* layout = new QVBoxLayout) { 
		widget_paradigm_ = new QWidget; 
		layout->setParent(widget_paradigm_);
		//layout->setContentsMargins(QMargins(3, 3, 3, 3));
		widget_layout_ = layout;
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
	
	QWidgetBuilder& 
	AddLayout(QLayout * layout) {
		if (!layout) return *this;
		dynamic_cast<QBoxLayout*>(widget_layout_)->addLayout(layout);
		return *this;
	}

	QWidgetBuilder& 
	AddLabel(const QString& text,	
			 const QString& obj_name	= "",	
			 const QString& qss			= "", 
			 const QSize &	mini_size	= QSize(0, 0), 
			 const QSize&	max_size	= QSize(16777215, 16777215), 
			 Qt::Alignment	align		= Qt::AlignLeft);	

	QWidgetBuilder& 
	AddLineEdit(const QString& text, 
				const QString& obj_name  = "", 
				const QString& qss		 = "",
				const QSize  & mini_size = QSize(0, 0),
				const QSize  & max_size	 = QSize(16777215, 16777215),
				Qt::Alignment  align	 = Qt::AlignLeft);

	QWidgetBuilder& AddSpacer(QSizePolicy::Policy h_policy, QSizePolicy::Policy v_policy) {
		widget_paradigm_->layout()->addItem(new QSpacerItem(40, 20, h_policy, v_policy));
		return *this;
	}

	inline QLayout* GetWidgetLayout() const { return widget_layout_; }

private:
	bool      is_returned_		= false;
	QWidget * widget_paradigm_	= nullptr;
	QLayout * widget_layout_	= nullptr;

	
};
}

#endif // !_WIDGET_BUILDER_H
