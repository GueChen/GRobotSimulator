#include "ui/widget/line_edit/drag_accept_edit.h"
#include "base/global/global_qss.h"

#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>

namespace GComponent {
DragAcceptorEditor::DragAcceptorEditor(QWidget*		 parent, 
									   DragEnterFunc enter_func,
									   DropFunc		 drop_func):
	DragAcceptorEditor("", parent, enter_func, drop_func)
{}

DragAcceptorEditor::DragAcceptorEditor(const QString& text, 
									   QWidget* parent, 
									   DragEnterFunc enter_func, 
									   DropFunc drop_func):
	QLineEdit(text, parent),
	drop_callback_(drop_func),
	drag_enter_callback_(enter_func)
{
	setStyleSheet(component_inspector_editor.data());
	setAlignment(Qt::AlignCenter);
	setReadOnly(true);
}

void DragAcceptorEditor::dropEvent(QDropEvent* event)
{
	drop_callback_(event);
}
void DragAcceptorEditor::dragEnterEvent(QDragEnterEvent* event)
{
	if (!drag_enter_callback_(event)) return;
	event->acceptProposedAction();
}
}