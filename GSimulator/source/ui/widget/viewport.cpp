#include "viewport.h"

#include "manager/resourcemanager.h"
#include "manager/modelmanager.h"
#include "manager/rendermanager.h"
#include "manager/physicsmanager.h"
#include "manager/planningmanager.h"
#include "manager/tcpsocketmanager.h"
#include "manager/objectmanager.h"
#include "system/collisionsystem.h"

#include "render/rhi/rhi_factory.h"
#include "function/adapter/modelloader_qgladapter.h"

#include "component/material_component.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#include <QtCore/QDir>
#include <QtCore/QmetaType>
#include <QtCore/QMimeData>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QOpenGLContext>
#include <QtCore/QThreadPool>
#include <QtWidgets/QVBoxLayout>

#include <cstdlib>
#include <regex>
#include <iostream>
#include <string_view>

#ifdef _DEBUG
#include <format>
#include <QDebug>
#endif
#include "component/collider_component.h"

//_____________________________________Test Usage____________________________________________
#include "model/robot/dual_arm_platform.h"
#include "model/robot/aubo_i3_model.h"
#include "component/transform_component.h"


static void CreateSphereObstacle(float x, float y, float z, float radius) {
    using namespace GComponent;
    static int idx = 0;
    static const std::string obj_name = "sphere";
    ObjectManager::getInstance().CreateInstance(obj_name);
    Model* sphere = ModelManager::getInstance().GetModelByName(obj_name + std::to_string(idx++));	
	sphere->GetTransform()->SetTransLocal(Vec3(x, y, z));
    sphere->GetTransform()->SetScale(Vec3::Ones()* radius);
    auto col_com = sphere->GetComponent<ColliderComponent>();
    col_com->RegisterShape(new SphereShape(0.5f * radius));

}

static void CreateCubeObstacle(float x, float y, float z, float x_l, float y_l, float z_l) {
    using namespace GComponent;
    static int idx = 0;
    static const std::string obj_name = "cube";
    ObjectManager::getInstance().CreateInstance(obj_name);
    Model* box = ModelManager::getInstance().GetModelByName(obj_name + std::to_string(idx++));
		
    box->GetTransform()->SetTransLocal(Vec3(x, y, z));
    box->GetTransform()->SetScale(Vec3(x_l, y_l, z_l));
    auto col_com = box->GetComponent<ColliderComponent>();
    col_com->RegisterShape(new BoxShape(x_l * 0.5f, y_l * 0.5f, z_l * 0.5f));
}

static void CreateCapsuleObstacle(float x, float y, float z, float radius, float half_z) {
    using namespace GComponent;
    static int idx = 0;
    static const std::string obj_name = "capsule";
    ObjectManager::getInstance().CreateInstance(obj_name);
    Model* capsule = ModelManager::getInstance().GetModelByName(obj_name + std::to_string(idx++));	

    capsule->GetTransform()->SetTransLocal(Vec3(x, y, z));
    capsule->GetTransform()->SetScale(Vec3(radius / 0.3, radius / 0.3, (half_z/ 0.7 + radius / 0.3) * 0.5f));
    auto col_com = capsule->GetComponent<ColliderComponent>();
    col_com->RegisterShape(new CapsuleShape(radius, half_z));
}

static void SceneInitialize() {
	GComponent::DUAL_ARM_PLATFORM platform;
	new GComponent::AUBO_I3_MODEL(nullptr);
	//CreateCubeObstacle(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	//CreateSphereObstacle(0.0f, 0.0f, 0.0f, 1.0f);
	//CreateCubeObstacle(0.5f, 0.8f, 1.0f, 0.2f, 0.7f, 0.3f);
	//CreateCubeObstacle(-2.5f, -1.8f, 0.0f, 0.4f, 0.3f, 0.5f);
	//CreateSphereObstacle(0.35f, 0.45f, 2.5f, 0.8f);
	//
	//CreateCubeObstacle(-0.30f, -0.4f, 1.2f, 0.5f, 0.5f, 0.5f);
	//CreateCapsuleObstacle(0.55f, -0.5f, 0.0f, 0.1f, 0.3f);
	//CreateCapsuleObstacle(0.0f, 0.0f, 0.0f, 0.3f, 0.7f);
	//CreateSphereObstacle(0.0f, 0.0f, 0.0f, 1.0f);
	//CreateCubeObstacle(0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);
}

static bool scene_initialize = false;

//_____________________________________Test Usage____________________________________________

namespace GComponent {

namespace {

QSurfaceFormat CreateViewportSurfaceFormat()
{
	QSurfaceFormat format;
	format.setVersion(4, 5);
	format.setSwapInterval(0);
	format.setSamples(4);
	return format;
}

RhiPresentSurfaceDesc CreateViewportPresentSurfaceDesc(const QWidget& viewport, RhiBackendType backend)
{
	RhiPresentSurfaceDesc surface_desc;
#ifdef _WIN32
	surface_desc.native_surface.type = RhiNativeSurfaceType::Win32Hwnd;
	surface_desc.native_surface.handle = reinterpret_cast<void*>(viewport.winId());
	const UINT dpi = GetDpiForWindow(reinterpret_cast<HWND>(viewport.winId()));
	surface_desc.width = static_cast<uint32_t>(MulDiv(viewport.width(), dpi, 96));
	surface_desc.height = static_cast<uint32_t>(MulDiv(viewport.height(), dpi, 96));
#else
	surface_desc.width = static_cast<uint32_t>(viewport.width());
	surface_desc.height = static_cast<uint32_t>(viewport.height());
#endif
	surface_desc.default_render_target_ownership = backend == RhiBackendType::DirectX12
		? RhiDefaultRenderTargetOwnership::Backend
		: RhiDefaultRenderTargetOwnership::External;
	return surface_desc;
}

RhiBackendType ParseRequestedViewportBackend()
{
	const char* raw_backend = std::getenv("GSIM_RHI_BACKEND");
	if (!raw_backend) {
		return RhiBackendType::OpenGL;
	}

	const std::string_view backend(raw_backend);
	if (backend == "dx12" || backend == "DX12" || backend == "d3d12" || backend == "D3D12") {
		return RhiBackendType::DirectX12;
	}

	return RhiBackendType::OpenGL;
}

const char* ToString(RhiBackendType backend)
{
	switch (backend) {
	case RhiBackendType::DirectX12:
		return "DirectX12";
	case RhiBackendType::OpenGL:
	default:
		return "OpenGL";
	}
}

}

RhiBackendType Viewport::GetDefaultRequestedBackend()
{
	return ParseRequestedViewportBackend();
}

Viewport::Viewport(QWidget* parent) :
	QOpenGLWidget(parent),
	ui_state_(width(), height()),
	requested_backend_(GetDefaultRequestedBackend())
{
	qRegisterMetaType<Viewport>("viewport");
	setFocusPolicy(Qt::StrongFocus);
	ConfigureRenderSurface();
	setAcceptDrops(true);

	render_timer_.setTimerType(Qt::CoarseTimer);
	render_timer_.setInterval(8);
	connect(&render_timer_, &QTimer::timeout, this, [this]() {
		update();
	});
	render_timer_.start();
}

Viewport::~Viewport() {}

NativeViewport::NativeViewport(QWidget* parent) :
	QWidget(parent),
	ui_state_(width(), height()),
	requested_backend_(Viewport::GetDefaultRequestedBackend())
{
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
	setAcceptDrops(true);

	render_timer_.setTimerType(Qt::CoarseTimer);
	render_timer_.setInterval(8);
	connect(&render_timer_, &QTimer::timeout, this, [this]() {
		update();
		});
	render_timer_.start();
}

NativeViewport::~NativeViewport() = default;

void NativeViewport::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	if (surface_initialized_) {
		return;
	}

	InitializeRenderSurface();
	surface_initialized_ = true;
}

void NativeViewport::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	ResizeRenderSurface(event->size().width(), event->size().height());
}

void NativeViewport::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);
	if (!surface_initialized_) {
		return;
	}

	RenderFrame();
}

ViewportHost::ViewportHost(QWidget* parent) :
	QWidget(parent),
	requested_backend_(Viewport::GetDefaultRequestedBackend())
{
	auto* host_layout = new QVBoxLayout(this);
	host_layout->setContentsMargins(0, 0, 0, 0);
	host_layout->setSpacing(0);

	auto* surface_container = new QWidget(this);
	surface_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	host_layout->addWidget(surface_container);

	auto* surface_layout = new QVBoxLayout(surface_container);
	surface_layout->setContentsMargins(0, 0, 0, 0);
	surface_layout->setSpacing(0);

	if (requested_backend_ == RhiBackendType::OpenGL) {
		viewport_widget_ = new Viewport(surface_container);
		render_surface_host_ = viewport_widget_;
		surface_layout->addWidget(viewport_widget_);
		connect(viewport_widget_, &Viewport::EmitDeltaTime, this, &ViewportHost::EmitDeltaTime);
		return;
	}

	native_viewport_widget_ = new NativeViewport(surface_container);
	render_surface_host_ = native_viewport_widget_;
	render_surface_host_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	surface_layout->addWidget(render_surface_host_);
	connect(native_viewport_widget_, &NativeViewport::EmitDeltaTime, this, &ViewportHost::EmitDeltaTime);
	std::cout << "Viewport host reserved native surface for backend: " << ToString(requested_backend_) << '\n';
}

ViewportHost::~ViewportHost() = default;

UIState* ViewportHost::GetUIState() const
{
	if (viewport_widget_) {
		return &viewport_widget_->ui_state_;
	}
	return native_viewport_widget_ ? native_viewport_widget_->GetUIState() : nullptr;
}


void Viewport::initializeGL()
{
	InitializeRenderSurface();
}

void Viewport::resizeGL(int w, int h)
{
	ResizeRenderSurface(w, h);
}

void Viewport::paintGL()
{
	RenderFrame();
}

void Viewport::ConfigureRenderSurface()
{
	if (requested_backend_ == RhiBackendType::OpenGL) {
		setFormat(CreateViewportSurfaceFormat());
	}
}

void Viewport::EnsureRhiDevice()
{
	if (!rhi_device_) {
		RhiDeviceInitConfig init_config;
		init_config.backend = requested_backend_;
#ifdef _DEBUG
		init_config.enable_debug_layer = init_config.backend == RhiBackendType::DirectX12;
		init_config.enable_validation = init_config.backend == RhiBackendType::DirectX12;
#endif

		rhi_device_ = CreateRhiDevice(init_config);
		if (!rhi_device_) {
			std::cerr << "CreateRhiDevice failed for requested backend "
				<< ToString(init_config.backend)
				<< ", falling back to OpenGL.\n";
			rhi_device_ = CreateOpenGLRhiDevice();
		}

		if (rhi_device_) {
			std::cout << "Viewport RHI backend: " << ToString(rhi_device_->GetBackendType()) << '\n';
		}
	}
}

void NativeViewport::EnsureRhiDevice()
{
	if (!rhi_device_) {
		RhiDeviceInitConfig init_config;
		init_config.backend = requested_backend_;
#ifdef _DEBUG
		init_config.enable_debug_layer = init_config.backend == RhiBackendType::DirectX12;
		init_config.enable_validation = init_config.backend == RhiBackendType::DirectX12;
#endif

		rhi_device_ = CreateRhiDevice(init_config);
		if (!rhi_device_) {
			std::cerr << "CreateRhiDevice failed for requested backend "
				<< ToString(init_config.backend)
				<< ", falling back to OpenGL.\n";
			rhi_device_ = CreateOpenGLRhiDevice();
		}

		if (rhi_device_) {
			std::cout << "Viewport RHI backend: " << ToString(rhi_device_->GetBackendType()) << '\n';
		}
	}
}

void Viewport::InitializeRenderSurface()
{
	EnsureRhiDevice();
	rhi_device_->InitializeForSurface(rhi_device_->GetInitConfig(), CreateViewportPresentSurfaceDesc(*this, rhi_device_->GetBackendType()));
	
	RegisteredShader();
	if (!camera_handle)
		camera_handle = GComponent::ModelManager::getInstance().RegisteredCamera();
	
	ui_state_.SetRhiDevice(rhi_device_);	
	GComponent::ResourceManager::getInstance().SetRhiDevice(rhi_device_);
	GComponent::RenderManager::getInstance().SetRhiDevice(rhi_device_);

	rhi_device_->Enable(RhiCapability::Multisample);

// Test USage
	if (not scene_initialize) {
		SceneInitialize();
		scene_initialize = true;
	}

// Test Usage
}

void NativeViewport::InitializeRenderSurface()
{
	EnsureRhiDevice();
	rhi_device_->InitializeForSurface(rhi_device_->GetInitConfig(), CreateViewportPresentSurfaceDesc(*this, rhi_device_->GetBackendType()));

	RegisteredShader();
	if (!camera_handle)
		camera_handle = GComponent::ModelManager::getInstance().RegisteredCamera();

	ui_state_.SetRhiDevice(rhi_device_);
	GComponent::ResourceManager::getInstance().SetRhiDevice(rhi_device_);
	GComponent::RenderManager::getInstance().SetRhiDevice(rhi_device_);

	rhi_device_->Enable(RhiCapability::Multisample);

	if (not scene_initialize) {
		SceneInitialize();
		scene_initialize = true;
	}
}

void Viewport::ResizeRenderSurface(int w, int h)
{
#ifdef WIN32
	UINT dpi = GetDpiForWindow(reinterpret_cast<HWND>(winId()));
	const int pixel_width = MulDiv(w, dpi, 96);
	const int pixel_height = MulDiv(h, dpi, 96);
	ui_state_.OnResize(pixel_width, pixel_height);
	if (rhi_device_) {
		rhi_device_->ResizePresentSurface(static_cast<uint32_t>(pixel_width), static_cast<uint32_t>(pixel_height));
	}
#else
	ui_state_.OnResize(w, h);
	if (rhi_device_) {
		rhi_device_->ResizePresentSurface(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
	}
#endif
}

void NativeViewport::ResizeRenderSurface(int w, int h)
{
#ifdef WIN32
	UINT dpi = GetDpiForWindow(reinterpret_cast<HWND>(winId()));
	const int pixel_width = MulDiv(w, dpi, 96);
	const int pixel_height = MulDiv(h, dpi, 96);
	ui_state_.OnResize(pixel_width, pixel_height);
	if (rhi_device_) {
		rhi_device_->ResizePresentSurface(static_cast<uint32_t>(pixel_width), static_cast<uint32_t>(pixel_height));
	}
#else
	ui_state_.OnResize(w, h);
	if (rhi_device_) {
		rhi_device_->ResizePresentSurface(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
	}
#endif
}

void Viewport::RenderFrame()
{
	using namespace glm;
	using namespace GComponent;
	static std::chrono::time_point last_point = std::chrono::steady_clock::now();
	static float delta = 0.0f;
	
	/*_________________________________Paint Main Loop________________________________________________________________________________________________*/
	Camera* camera_ptr = ModelManager::getInstance().GetCameraByHandle(camera_handle);
	// Set global Render parameters
	RenderGlobalInfo& render_info = RenderManager::getInstance().m_render_sharing_msg;
	render_info.SetSimpleDirLight(vec3(0.5f, 1.0f, 1.0f), vec3(1.0f));
	render_info.SetCameraInfo(*camera_ptr);
	render_info.SetProjectionPlane(0.001f, 1000.0f);
	render_info.UpdateProjectionMatrix();
	
	// custom definition update method
	CustomUpdateImpl();

	// Process Input
	ui_state_.tick();
	
	// Adjust the planning
	PlanningManager::getInstance().tick(delta_time.count());
	// Sync the frame
	TcpSocketManager::getInstance().tick();
	// Adjust all component
	ModelManager::getInstance().tickAll(delta_time.count());
	// Adjust all resources
	ResourceManager::getInstance().tick(rhi_device_);
	// Process all collision Event
	CollisionSystem::getInstance().tick(delta_time.count());
	// Adjust all the physics actors
	PhysicsManager::getInstance().tick(delta_time.count());
	// Draw all renderable process Passes
	RenderManager::getInstance().tick();
	
	// 
	// Time statics rendering over
	std::chrono::time_point now = std::chrono::steady_clock::now();
	delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_point);
	last_point = now;
	rhi_device_->Present();
	emit EmitDeltaTime(delta_time.count());
}

void NativeViewport::RenderFrame()
{
	using namespace glm;
	using namespace GComponent;
	static std::chrono::time_point last_point = std::chrono::steady_clock::now();
	static float delta = 0.0f;

	Camera* camera_ptr = ModelManager::getInstance().GetCameraByHandle(camera_handle);
	RenderGlobalInfo& render_info = RenderManager::getInstance().m_render_sharing_msg;
	render_info.SetSimpleDirLight(vec3(0.5f, 1.0f, 1.0f), vec3(1.0f));
	render_info.SetCameraInfo(*camera_ptr);
	render_info.SetProjectionPlane(0.001f, 1000.0f);
	render_info.UpdateProjectionMatrix();

	ui_state_.tick();
	PlanningManager::getInstance().tick(delta_time.count());
	TcpSocketManager::getInstance().tick();
	ModelManager::getInstance().tickAll(delta_time.count());
	ResourceManager::getInstance().tick(rhi_device_);
	CollisionSystem::getInstance().tick(delta_time.count());
	PhysicsManager::getInstance().tick(delta_time.count());
	RenderManager::getInstance().tick();

	std::chrono::time_point now = std::chrono::steady_clock::now();
	delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_point);
	last_point = now;
	rhi_device_->Present();
	emit EmitDeltaTime(delta_time.count());
}

void Viewport::CustomUpdateImpl()
{
	
}

/*________________________________Events Implementations_____________________________________________*/
void Viewport::keyPressEvent(QKeyEvent* event)
{
	GComponent::Camera* camera_ptr = GComponent::ModelManager::getInstance().GetCameraByHandle(camera_handle);
	size_t key_state = 0x0;
	if (event->key() == Qt::Key_W) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyW);
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::FORWARD, delta_time.count());
	}
	if (event->key() == Qt::Key_S) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyS);
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::BACKWARD, delta_time.count());
	}
	if (event->key() == Qt::Key_A) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyA);
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::LEFT, delta_time.count());
	}
	if (event->key() == Qt::Key_D) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyD);
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::RIGHT, delta_time.count());
	}
	if (event->key() == Qt::Key_Q) {
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::UP, delta_time.count());
	}
	if (event->key() == Qt::Key_E) {
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::DOWN, delta_time.count());
	}

	if (event->key() == Qt::Key_Delete) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyDelete);		
	}

	ui_state_.OnKeyPress(key_state);
}

void Viewport::keyReleaseEvent(QKeyEvent* event)
{
	size_t key_state = 0x0;
	if (event->key() == Qt::Key_W) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyW);
	}
	if (event->key() == Qt::Key_S) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyS);
	}
	if (event->key() == Qt::Key_A) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyA);
	}
	if (event->key() == Qt::Key_D) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyD);
	}
	if (event->key() == Qt::Key_Delete) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyDelete);
	}

	ui_state_.OnKeyRelease(key_state);
}

void Viewport::mouseMoveEvent(QMouseEvent* event)
{
#ifdef WIN32
	int dpi = GetDpiForWindow(reinterpret_cast<HWND>(winId()));
	ui_state_.OnCursorMove(MulDiv(event->x(), dpi, 96), MulDiv(event->y(), dpi, 96));
#else
	ui_state_.OnCursorMove(event->x(), event->y());
#endif // WIN32

	if (!ui_state_.GetIsDraged())
	{		
		QPoint cur_point = event->pos();
		GComponent::Camera* camera_ptr = GComponent::ModelManager::getInstance().GetCameraByHandle(camera_handle);
		int horizon_diff  = cur_point.x() - mouse_pressed_last_pos_.x(),
			vertical_diff = -cur_point.y() + mouse_pressed_last_pos_.y();
		if (event->buttons() & Qt::MouseButton::RightButton) {
			camera_ptr->ProcessMouseMovement(horizon_diff, vertical_diff);				
		}
		if (event->buttons() & Qt::MouseButton::MiddleButton) {
			camera_ptr->Move(horizon_diff * 0.02f, vertical_diff * 0.02f, 0.0f);	
		}
		mouse_pressed_last_pos_ = cur_point;		
	}
}

void Viewport::mousePressEvent(QMouseEvent* event)
{
#ifdef WIN32
	int dpi = GetDpiForWindow(reinterpret_cast<HWND>(winId()));
	ui_state_.OnCursorMove(MulDiv(event->x(), dpi, 96), MulDiv(event->y(), dpi, 96));
#else
	ui_state_.OnCursorMove(event->x(), event->y());
#endif // WIN32

	ui_state_.OnMousePress(event->buttons());
	
	if (!ui_state_.GetIsDraged()) {
		if (event->button() & (Qt::MouseButton::RightButton | Qt::MouseButton::MiddleButton))
		{
			mouse_pressed_last_pos_ = event->pos();
		}	
	}
}

void Viewport::mouseReleaseEvent(QMouseEvent* event)
{
	ui_state_.OnMouseRelease(event->button());
}

void Viewport::enterEvent(QEnterEvent* event)
{
#ifdef WIN32
	int dpi = GetDpiForWindow(reinterpret_cast<HWND>(winId()));
	ui_state_.OnMouseEnter(MulDiv(static_cast<int>(event->position().x()), dpi, 96),
						   MulDiv(static_cast<int>(event->position().y()), dpi, 96));
#else
	ui_state_.OnMouseEnter(event->x(), event->y());
#endif // WIN32
	setMouseTracking(true);
}

void Viewport::leaveEvent(QEvent* event)
{
	ui_state_.OnMouseLeave();
	setMouseTracking(false);
}

void Viewport::wheelEvent(QWheelEvent* event)
{
	int delta = event->angleDelta().y();
	ui_state_.OnWheel(delta);
	if (!ui_state_.GetIsDraged()) {
		GComponent::Camera* camera_ptr = GComponent::ModelManager::getInstance().GetCameraByHandle(camera_handle);
		camera_ptr->Move(0.0f, 0.0f, delta / 1200.0f);
	}
	event->accept();
}

/*________________________________Drags and Drops relative_____________________________________________*/
void Viewport::dropEvent(QDropEvent* event)
{
	for (auto&& url : event->mimeData()->urls()) {
		QFileInfo dir(url.path().mid(1));
		std::cout << url.toString().toStdString() << std::endl;		
		std::cout << dir.baseName().toStdString() << std::endl;
		
		QThreadPool::globalInstance()->start([dir = dir]() {
			try {
				std::string mesh_name = dir.baseName().toStdString();				
				RenderMesh* mesh_ptr  = ResourceManager::getInstance().GetMeshByName(mesh_name);

				if (!mesh_ptr) {	// register mesh resource
					mesh_ptr = QGL::ModelLoader::getMeshPtr(dir.filePath().toStdString());
					if (mesh_ptr) {
						ResourceManager::getInstance().RegisteredMesh(mesh_name, mesh_ptr);
					}
				}

				if (mesh_ptr) {		// register model
					std::string obj_name = mesh_name;
					int number = 1;
					while (ModelManager::getInstance().GetModelByName(obj_name)) {
						std::regex regex("\\d+$");
						std::string new_name = std::regex_replace(obj_name, regex, std::to_string(number));
						if (new_name == obj_name) {
							new_name.append("_" + std::to_string(number));
						}
						obj_name = new_name;
						++number;
					}
					Model* model = new Model(obj_name, mesh_name, GComponent::Mat4::Identity(), nullptr);
					model->RegisterComponent(std::make_unique<MaterialComponent>(model, "color", true));
					ModelManager::getInstance().RegisteredModel(obj_name, model);		
				}
			}
			catch (std::ifstream::failure e) {
				std::cerr << "Loading Failed, consider the file is not a mesh file\n";
			}
			});
	}
}

void Viewport::dragEnterEvent(QDragEnterEvent* event)
{
	event->acceptProposedAction();
}

void Viewport::RegisteredShader()
{
	auto register_shader = [](const char* name, const char* vert, const char* frag, const char* geom = "") {
		ResourceManager::getInstance().RegisteredShader(MakeOpenGlShaderDesc(name, vert, frag, geom));
	};

	register_shader("skybox",				PathVert(skybox),				PathFrag(skybox));
	register_shader("postprocess",			PathVert(postprocess),			PathFrag(postprocess));
	register_shader("outline",				PathVert(outline),				PathFrag(outline));
	register_shader("infinite_grid",		PathVert(infinite_grid),		PathFrag(infinite_grid));
	register_shader("deferred_geometry",	PathVert(deferred_geometry),	PathFrag(deferred_geometry));
	register_shader("deferred_lighting",	PathVert(deferred_lighting),	PathFrag(deferred_lighting));
	register_shader("deferred_depth",		PathVert(deferred_depth),		PathFrag(deferred_depth));
	register_shader("no_shadow_color",		PathVert(Color),				PathFrag(Color));
	register_shader("axis",					PathVert(axis),					PathFrag(axis));
	register_shader("picking",				PathVert(picking),				PathFrag(picking));
	register_shader("base",					PathVert(Base),					PathFrag(Base));
	register_shader("linecolor",			PathVert(LineColor),			PathFrag(LineColor));
	register_shader("depth_map",			PathVert(depthOrtho),			PathFrag(depthOrtho));
	register_shader("shadow_color",			PathVert(shadowOrtho),			PathFrag(shadowOrtho));
	register_shader("csm_depth_map",		PathVert(csm_depth_ortho),		PathFrag(csm_depth_ortho),		PathGeom(csm_depth_ortho));
	register_shader("color",				PathVert(cascade_shadow_ortho),	PathFrag(cascade_shadow_ortho));
	// pbr relative
	register_shader("pbr",					PathVert(pbr),					PathFrag(pbr));
	register_shader("equirectangular2cube", PathVert(cube),					PathFrag(equirectangular2cubemap));
	register_shader("irr_conv",				PathVert(skybox),				PathFrag(irradiance_conv));
	register_shader("pft_conv",				PathVert(skybox),				PathFrag(prefilter_conv));
	register_shader("brdf_lut",				PathVert(brdf_lut),				PathFrag(brdf_lut));
}

void NativeViewport::RegisteredShader()
{
	auto register_shader = [](const char* name, const char* vert, const char* frag, const char* geom = "") {
		ResourceManager::getInstance().RegisteredShader(MakeOpenGlShaderDesc(name, vert, frag, geom));
	};

	register_shader("skybox",				PathVert(skybox),				PathFrag(skybox));
	register_shader("postprocess",			PathVert(postprocess),			PathFrag(postprocess));
	register_shader("outline",				PathVert(outline),				PathFrag(outline));
	register_shader("infinite_grid",		PathVert(infinite_grid),		PathFrag(infinite_grid));
	register_shader("deferred_geometry",	PathVert(deferred_geometry),	PathFrag(deferred_geometry));
	register_shader("deferred_lighting",	PathVert(deferred_lighting),	PathFrag(deferred_lighting));
	register_shader("deferred_depth",		PathVert(deferred_depth),		PathFrag(deferred_depth));
	register_shader("no_shadow_color",		PathVert(Color),				PathFrag(Color));
	register_shader("axis",					PathVert(axis),					PathFrag(axis));
	register_shader("picking",				PathVert(picking),				PathFrag(picking));
	register_shader("base",					PathVert(Base),					PathFrag(Base));
	register_shader("linecolor",			PathVert(LineColor),			PathFrag(LineColor));
	register_shader("depth_map",			PathVert(depthOrtho),			PathFrag(depthOrtho));
	register_shader("shadow_color",			PathVert(shadowOrtho),			PathFrag(shadowOrtho));
	register_shader("csm_depth_map",		PathVert(csm_depth_ortho),		PathFrag(csm_depth_ortho),		PathGeom(csm_depth_ortho));
	register_shader("color",				PathVert(cascade_shadow_ortho),	PathFrag(cascade_shadow_ortho));
	register_shader("pbr",					PathVert(pbr),					PathFrag(pbr));
	register_shader("equirectangular2cube", PathVert(cube),					PathFrag(equirectangular2cubemap));
	register_shader("irr_conv",				PathVert(skybox),				PathFrag(irradiance_conv));
	register_shader("pft_conv",				PathVert(skybox),				PathFrag(prefilter_conv));
	register_shader("brdf_lut",				PathVert(brdf_lut),				PathFrag(brdf_lut));
}

}
