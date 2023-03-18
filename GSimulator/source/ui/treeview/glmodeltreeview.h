/**
 *  @file  	glmodeltreeview.h
 *  @brief 	Display all the model in the manager and provide Some Manager interface.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 11th, 2022
 **/
#ifndef _GLMODELTREEVIEW_H
#define _GLMODELTREEVIEW_H

#include <QtWidgets/QTreeView>
#include <QtWidgets/QMenu>
#include <QtCore/QString>

#include <string>

namespace Ui { class GLModelTreeView; };


namespace GComponent 
{
using std::string;
class GLModelTreeView : public QTreeView
{
	Q_OBJECT	
public:
	GLModelTreeView(QWidget* parent = nullptr);
	~GLModelTreeView();

	void setModel(QAbstractItemModel* model)   override;
		
protected:
	void mouseReleaseEvent(QMouseEvent* event)	  override;
	void keyReleaseEvent  (QKeyEvent* event)	  override;
	
	void dragMoveEvent	  (QDragMoveEvent* event) override;
	void dragEnterEvent	  (QDragEnterEvent* event)override;
	void dropEvent		  (QDropEvent* evnet)	  override;
private:
	void InitMenuActions();

public slots:
	void ResponseSelectRequest(const QModelIndex& select);	
	void ResponseDataDeleted();

private slots:
	void SelectionChangeSlot(const QModelIndex& current, const QModelIndex& previous);
	
signals:
	void RobotCreateRequest();
	void MorphIntoConvexRequest();
	void CreateRequest(const string& name);
	void SelectRequest(const string& name);
	void DeleteRequest(const string& name);

private:
	QMenu				 m_basic_menu;
/*_______________________BASIC MENU ACTIONS_______________________________*/	
	QAction*			 copy_action_;			/* copy					  */
	QAction*			 cut_action_;			/* cut					  */
	QAction*			 paste_action_;			/* paste				  */
												/*_______________________ */
	QAction*			 delete_action_;		/* delete				  */
	QAction*			 rename_action_;		/* rename				  */
											    /*_______________________ */
	QMenu				 m_add_menu;			/* 3D Objects |>		  */	
	QAction*			 robot_create_action_;	/* robot				  */
/*________________________________________________________________________*/
	QMenu				 m_edit_menu;			/* Edit					  */	
/*________________________________________________________________________*/
	
/*_______________________ADD MENU ACTIONS_________________________________*/
	QAction*			 cube_create_action_;
	QAction*			 sphere_create_action_;
	QAction*			 cylinder_create_action_;
	QAction*			 capsule_create_action_;
	QAction*			 plane_create_action_;
/*________________________________________________________________________*/
	
/*_______________________Edit Menu Actions________________________________*/
	QAction*			 morph_into_convex_decompositions_;
	QAction*			 morph_into_convex_hull_;
/*________________________________________________________________________*/
};

}

#endif // !_GLMODELTREEVIEW_H


