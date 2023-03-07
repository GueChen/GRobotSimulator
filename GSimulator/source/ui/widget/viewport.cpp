#include "viewport.h"

#include "manager/resourcemanager.h"
#include "manager/modelmanager.h"
#include "manager/rendermanager.h"
#include "manager/physicsmanager.h"
#include "manager/planningmanager.h"
#include "manager/tcpsocketmanager.h"

#include "function/adapter/modelloader_qgladapter.h"

#include "component/material_component.h"

#include <QtCore/QDir>
#include <QtCore/QmetaType>
#include <QtCore/QMimeData>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QKeyEvent>
#include <QtCore/QThreadPool>

#include <regex>

#ifdef _DEBUG
#include <iostream>
#include <format>
#include <QDebug>
#endif
#include "component/collider_component.h"

#include <GComponent/Geometry/gcollision_detection.h>

namespace GComponent {

Viewport::Viewport(QWidget* parent) :
	QOpenGLWidget(parent),
	ui_state_(width(), height()),
	gl_(std::make_shared<GComponent::MyGL>())
{
	qRegisterMetaType<Viewport>("viewport");
	setFocusPolicy(Qt::StrongFocus);
	QSurfaceFormat set_format;
	set_format.setSamples(4);
	set_format.setVersion(4, 5);
	setFormat(set_format);
	setAcceptDrops(true);
}

Viewport::~Viewport() {}


void Viewport::initializeGL()
{
	gl_->initializeOpenGLFunctions();
	
	RegisteredShader();
	if (!camera_handle)
		camera_handle = GComponent::ModelManager::getInstance().RegisteredCamera();
	
	ui_state_.SetGL(gl_);	
	GComponent::ResourceManager::getInstance().SetGL(gl_);
	GComponent::RenderManager::getInstance().SetGL(gl_);

	QOpenGLContext::currentContext()->format().setSwapInterval(0);
}

void Viewport::resizeGL(int w, int h)
{
	//gl_->glViewport(0.0f, 0.0f, w, h);
#ifdef WIN32
	UINT dpi = GetDpiForWindow(reinterpret_cast<HWND>(winId()));
	ui_state_.OnResize(MulDiv(w, dpi, 96), MulDiv(h, dpi, 96));
#else
	ui_state_.OnResize(w, h);
#endif
}

void Viewport::paintGL()
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
	ResourceManager::getInstance().tick(gl_);
	// Adjust all the physics actors
	PhysicsManager::getInstance().tick(delta_time.count());
	// Draw all renderable process Passes
	RenderManager::getInstance().tick();
	
	// 
	// Time statics rendering over
	std::chrono::time_point now = std::chrono::steady_clock::now();
	delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_point);
	last_point = now;
	emit EmitDeltaTime(delta_time.count());
	update();
}

void Viewport::CustomUpdateImpl()
{
	Model* cube0 = ModelManager::getInstance().GetModelByName("cube0");
	Model* cube1 = ModelManager::getInstance().GetModelByName("cube1");
	Model* sphere0 = ModelManager::getInstance().GetModelByName("sphere0");
	Model* capsule0 = ModelManager::getInstance().GetModelByName("capsule0");
	
	auto col_com0 = cube0->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
	auto col_com1 = cube1->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
	auto col_com2 = sphere0->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
	auto cap_com3 = capsule0->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());	

	auto shapes0 = col_com0->GetShapes();
	auto shapes1 = col_com1->GetShapes();
	auto shapes2 = col_com2->GetShapes();
	auto shape_cap0 = cap_com3->GetShapes();

	cube0->intesection_ = cube1->intesection_ 
						= sphere0->intesection_
						= capsule0->intesection_
						= false;

	for (auto& s0 : shapes0) {
		auto box0 = dynamic_cast<BoxShape*>(s0);
		for (auto& s1 : shapes1) {
			auto box1 = dynamic_cast<BoxShape*>(s1);
			if (IntersectOBBOBB(Vec3(box0->m_half_x, box0->m_half_y, box0->m_half_z), cube0->getTransGlobal(), cube0->getRotGlobal(),
				Vec3(box1->m_half_x, box1->m_half_y, box1->m_half_z), cube1->getTransGlobal(), cube1->getRotGlobal())) {
				cube0->intesection_ = true;
				cube1->intesection_ = true;
				goto boxcheckfinish;
			}
		}
	}

boxcheckfinish:
	for (auto& s0 : shapes0) {
		auto box0 = dynamic_cast<BoxShape*>(s0);
		for (auto& s1 : shapes2) {
			auto sshape = dynamic_cast<SphereShape*>(s1);
			if (IntersectOBBSphere(Vec3(box0->m_half_x, box0->m_half_y, box0->m_half_z), cube0->getTransGlobal(), cube0->getRotGlobal(),
				sshape->m_radius, sphere0->getTransGlobal())) {
				cube0->intesection_   = true;
				sphere0->intesection_ = true;				
			}
		}
	}

	for (auto& s0 : shapes1) {
		auto box0 = dynamic_cast<BoxShape*>(s0);
		for (auto& s1 : shapes2) {
			auto sshape = dynamic_cast<SphereShape*>(s1);
			if (IntersectOBBSphere(Vec3(box0->m_half_x, box0->m_half_y, box0->m_half_z), cube1->getTransGlobal(), cube1->getRotGlobal(),
				sshape->m_radius, sphere0->getTransGlobal())) {
				cube1->intesection_   = true;
				sphere0->intesection_ = true;				
			}
		}
	}

	for (auto& s0 : shape_cap0) {
		auto shape_cap0 = dynamic_cast<CapsuleShape*>(s0);
		for (auto& s1 : shapes0) {
			auto box_shape = dynamic_cast<BoxShape*>(s1);
			if (IntersectOBBCaspsule(Vec3(box_shape->m_half_x, box_shape->m_half_y, box_shape->m_half_z), cube0->getTransGlobal(), cube0->getRotGlobal(),
				shape_cap0->m_radius, shape_cap0->m_half_height, capsule0->getTransGlobal(), capsule0->getRotGlobal())) {
				cube0->intesection_ = true;
				capsule0->intesection_ = true;
			}
		}
	}
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
	ui_state_.OnMouseEnter(event->x(), event->y());
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
					model->RegisterComponent(std::make_unique<MaterialComponent>(model, "color"));
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
	ResourceManager::getInstance().RegisteredShader("skybox",				new MyShader(nullptr,	PathVert(skybox),				PathFrag(skybox)));
	ResourceManager::getInstance().RegisteredShader("postprocess",			new MyShader(nullptr,	PathVert(postprocess),			PathFrag(postprocess)));
	ResourceManager::getInstance().RegisteredShader("color",				new MyShader(nullptr,	PathVert(Color),				PathFrag(Color)));
    ResourceManager::getInstance().RegisteredShader("axis",					new MyShader(nullptr,	PathVert(axis),					PathFrag(axis)));
    ResourceManager::getInstance().RegisteredShader("picking",				new MyShader(nullptr,	PathVert(picking),				PathFrag(picking)));
    ResourceManager::getInstance().RegisteredShader("base",					new MyShader(nullptr,	PathVert(Base),					PathFrag(Base)));
    ResourceManager::getInstance().RegisteredShader("linecolor",			new MyShader(nullptr,	PathVert(LineColor),			PathFrag(LineColor)));
	ResourceManager::getInstance().RegisteredShader("depth_map",			new MyShader(nullptr,	PathVert(depthOrtho),			PathFrag(depthOrtho)));
	ResourceManager::getInstance().RegisteredShader("shadow_color",			new MyShader(nullptr,	PathVert(shadowOrtho),			PathFrag(shadowOrtho)));
	ResourceManager::getInstance().RegisteredShader("csm_depth_map",		new MyShader(nullptr,	PathVert(csm_depth_ortho),		PathFrag(csm_depth_ortho),	PathGeom(csm_depth_ortho)));
	ResourceManager::getInstance().RegisteredShader("cascade_shadow_ortho",	new MyShader(nullptr,	PathVert(cascade_shadow_ortho),	PathFrag(cascade_shadow_ortho)));
}

}