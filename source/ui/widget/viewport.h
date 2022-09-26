/**
 *  @file  	viewport.h
 *  @brief 	This class is use opengl function to create an observer environment UI bind with a manager UIState.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 19th, 2022
 **/
#ifndef _VIEWPORT_H
#define _VIEWPORT_H

#include "manager/editor/uistatemanager.h"

#include "render/mygl.hpp"
#include "render/myshader.h"
#include "render/camera.hpp"

#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtCore/QTimer>

#include <memory>

namespace GComponent {
	class Viewport : public QOpenGLWidget
	{
		Q_OBJECT
	public:
		explicit Viewport(QWidget* parent = nullptr);
		~Viewport();

	protected:
		void initializeGL() override;
		void resizeGL(int w, int h) override;
		void paintGL() override;
		void CustomUpdateImpl();

	private:
		void RegisteredShader();

		/// Event Definitions 事件定义
	protected:
		void keyPressEvent(QKeyEvent* event)		override;
		void keyReleaseEvent(QKeyEvent* event)		override;
		void mouseMoveEvent(QMouseEvent* event)		override;
		void mousePressEvent(QMouseEvent* event)	override;
		void mouseReleaseEvent(QMouseEvent* event)	override;
		void enterEvent(QEnterEvent* event)			override;
		void leaveEvent(QEvent* event)				override;

	signals:
		void EmitDeltaTime(float delta_time);
	
	public:
		UIState											ui_state_;

	private:		
		std::shared_ptr<MyGL>							gl_;

		QPoint											mouse_right_pressed;
		size_t										    camera_handle = 0;

		std::chrono::duration<float>					delta_time;
	};
}

#endif // !_VIEWPORT_H
