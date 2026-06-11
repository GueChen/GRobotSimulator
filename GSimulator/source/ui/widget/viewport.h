/**
 *  @file  	viewport.h
 *  @brief 	This class is use opengl function to create an observer environment UI bind with a manager UIState.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 19th, 2022
 **/
#ifndef _VIEWPORT_H
#define _VIEWPORT_H

#include "manager/editor/uistatemanager.h"

#include "render/rhi/rhi_device.h"
#include "render/myshader.h"
#include "render/camera.hpp"

#include <QtWidgets/QWidget>
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
		static RhiBackendType GetDefaultRequestedBackend();
		RhiBackendType GetRequestedBackend() const { return requested_backend_; }

	protected:
		void initializeGL() override;
		void resizeGL(int w, int h) override;
		void paintGL() override;
		void CustomUpdateImpl();

	private:
		void ConfigureRenderSurface();
		void EnsureRhiDevice();
		void InitializeRenderSurface();
		void ResizeRenderSurface(int w, int h);
		void RenderFrame();
		void RegisteredShader();

		/// Event definitions
	protected:
		void keyPressEvent(QKeyEvent* event)		override;
		void keyReleaseEvent(QKeyEvent* event)		override;
		void mouseMoveEvent(QMouseEvent* event)		override;
		void mousePressEvent(QMouseEvent* event)	override;
		void mouseReleaseEvent(QMouseEvent* event)	override;
		void enterEvent(QEnterEvent* event)			override;
		void leaveEvent(QEvent* event)				override;
		void wheelEvent(QWheelEvent* event)			override;
		
		// drag and drop
		void dropEvent(QDropEvent* event)			override;
		void dragEnterEvent(QDragEnterEvent* event) override;
	signals:
		void EmitDeltaTime(float delta_time);
	
	public:
		UIState											ui_state_;

	private:		
		std::shared_ptr<IRhiDevice>						rhi_device_;
		RhiBackendType									requested_backend_ = RhiBackendType::OpenGL;
		QTimer											render_timer_;

		QPoint											mouse_pressed_last_pos_;
		size_t										    camera_handle = 0;

		std::chrono::duration<float>					delta_time;
	};

	class NativeViewport : public QWidget
	{
		Q_OBJECT
	public:
		explicit NativeViewport(QWidget* parent = nullptr);
		~NativeViewport() override;
		RhiBackendType GetRequestedBackend() const { return requested_backend_; }
		UIState* GetUIState() { return &ui_state_; }

	protected:
		void showEvent(QShowEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;
		void paintEvent(QPaintEvent* event) override;

	signals:
		void EmitDeltaTime(float delta_time);

	private:
		void EnsureRhiDevice();
		void InitializeRenderSurface();
		void ResizeRenderSurface(int w, int h);
		void RenderFrame();
		void RegisteredShader();

	public:
		UIState											ui_state_;

	private:
		std::shared_ptr<IRhiDevice>						rhi_device_;
		RhiBackendType									requested_backend_ = RhiBackendType::DirectX12;
		QTimer											render_timer_;
		size_t											camera_handle = 0;
		std::chrono::duration<float>					delta_time;
		bool											surface_initialized_ = false;
	};

	class ViewportHost : public QWidget
	{
		Q_OBJECT
	public:
		explicit ViewportHost(QWidget* parent = nullptr);
		~ViewportHost() override;

		UIState* GetUIState() const;
		Viewport* GetViewportWidget() const { return viewport_widget_; }
		QWidget* GetRenderSurfaceHost() const { return render_surface_host_; }
		RhiBackendType GetRequestedBackend() const { return requested_backend_; }

	signals:
		void EmitDeltaTime(float delta_time);

	private:
		RhiBackendType									requested_backend_ = RhiBackendType::OpenGL;
		QWidget*										render_surface_host_ = nullptr;
		Viewport*										viewport_widget_ = nullptr;
		NativeViewport*									native_viewport_widget_ = nullptr;
	};
}

#endif // !_VIEWPORT_H
