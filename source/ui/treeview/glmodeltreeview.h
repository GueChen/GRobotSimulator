/**
 *  @file  	glmodeltreeview.h
 *  @brief 	Display all the model in the manager and provide Some Manager interface.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 11th, 2022
 **/
#ifndef _GLMODELTREEVIEW_H
#define _GLMODELTREEVIEW_H

#include "base/editortreemodel.h"

#include <QtWidgets/QTreeView>
#include <QtWidgets/QMenu>
#include <QtCore/QString>

namespace Ui { class GLModelTreeView; };

namespace GComponent 
{

class GLModelTreeView : public QTreeView
{
	Q_OBJECT
public:
	GLModelTreeView(QWidget* parent = nullptr);
	~GLModelTreeView();

protected:
	void mouseReleaseEvent(QMouseEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event)	   override;

private:
	QString GetTreeModelData();
	void InitMenuActions();
public slots:
	void ResponseSelectRequest(const string& name);
	void ResponseDeleteRequest(const string& name);

private slots:
	void SelectionChangeSlot(const QModelIndex& current, const QModelIndex& previous);
	
signals:
	void CreateRequest(const string& name);
	void SelectRequest(const string& name);
	void DeleteRequest(const string& name);

private:
	EditorTreeModel*	 m_tree_model;
	Ui::GLModelTreeView* ui;

	QMenu				 m_basic_menu;
/*_______________________BASIC MENU ACTIONS_______________________________*/	
	QAction*			 copy_action_;
	QAction*			 cut_action_;
	QAction*			 paste_action_;

	QAction*			 delete_action_;
	QAction*			 rename_action_;
	QMenu				 m_add_menu;	
/*________________________________________________________________________*/
	
/*_______________________ADD MENU ACTIONS_________________________________*/
	QAction*			 cube_create_action_;
	QAction*			 sphere_create_action_;
	QAction*			 cylinder_create_action_;
	QAction*			 capsule_create_action_;
	QAction*			 plane_create_action_;
/*________________________________________________________________________*/
	
};

}

#endif // !_GLMODELTREEVIEW_H


