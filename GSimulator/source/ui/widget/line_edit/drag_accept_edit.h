/**
 *  @file  	drag_accept_edit.h
 *  @brief  This UI is suuply for specialize display the resource, accept special resource type by drag and drop.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 8th, 2023
 **/
#ifndef __DRAG_ACCEPT_EDIT_H
#define __DRAG_ACCEPT_EDIT_H

#include <QtWidgets/QLineEdit>

#include <functional>

namespace GComponent {
	
class DragAcceptorEditor : public QLineEdit {
	Q_OBJECT
public:
	using DragEnterFunc = std::function<bool(QDragEnterEvent*)>;
	using DropFunc		= std::function<void(QDropEvent*)>;

public:
	explicit DragAcceptorEditor(QWidget*	  parent	 = nullptr, 
								DragEnterFunc enter_func = [](QDragEnterEvent*) { return false; },
								DropFunc	  drop_func	 = [](QDropEvent*){});
	explicit DragAcceptorEditor(const QString&, 
								QWidget*	  parent	 = nullptr, 
								DragEnterFunc enter_func = [](QDragEnterEvent*) { return false; },
								DropFunc	  drop_func	 = [](QDropEvent*){});

	~DragAcceptorEditor() = default;

protected:
	void dropEvent	   (QDropEvent* event)		override;
	void dragEnterEvent(QDragEnterEvent* event) override;

private:
	DropFunc	  drop_callback_;
	DragEnterFunc drag_enter_callback_;
};

}


#endif // !__DRAG_ACCEPT_EDIT_H
