#include "viewport.h"

#include "manager/resourcemanager.h"
#include "manager/modelmanager.h"
#include "manager/rendermanager.h"
#include "manager/physicsmanager.h"
#include "manager/planningmanager.h"
#include "manager/tcpsocketmanager.h"
#include "manager/objectmanager.h"
#include "system/collisionsystem.h"

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

//_____________________________________Test Usage____________________________________________
#include "model/robot/dual_arm_platform.h"

static void SetPBRRandomProperties(GComponent::Model* model) {
	using namespace GComponent;
	auto get_random = []()->float {
		return rand() % 10000 / 10000.0f;
	};
	auto& material = *model->GetComponent<MaterialComponent>(MaterialComponent::type_name.data());
	auto& properties = material.GetProperties();
	for (auto& prop : properties) {
		std::cout << prop.name << std::endl;
		if (prop.name == "albedo color") {
			prop.val = glm::vec3(get_random(), get_random(), get_random());
		}
		else if (prop.name == "ao") {
			prop.val = get_random();
		}
		else if (prop.name == "metallic") {
			prop.val = get_random();
		}
		else if (prop.name == "roughness") {
			prop.val = get_random();
		}
	}	

}

static void CreateSphereObstacle(float x, float y, float z, float radius) {
    using namespace GComponent;
    static int idx = 0;
    static const std::string obj_name = "sphere";
    ObjectManager::getInstance().CreateInstance(obj_name);
    Model* sphere = ModelManager::getInstance().GetModelByName(obj_name + std::to_string(idx++));
	
	SetPBRRandomProperties(sphere);

	sphere->setTransLocal(Vec3(x, y, z));
    sphere->setScale(Vec3::Ones()* radius);
    sphere->RegisterComponent(std::make_unique<ColliderComponent>(sphere));
    auto col_com = sphere->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
    col_com->RegisterShape(new SphereShape(0.5f * radius));

}

static void CreateCubeObstacle(float x, float y, float z, float x_l, float y_l, float z_l) {
    using namespace GComponent;
    static int idx = 0;
    static const std::string obj_name = "cube";
    ObjectManager::getInstance().CreateInstance(obj_name);
    Model* box = ModelManager::getInstance().GetModelByName(obj_name + std::to_string(idx++));
	
	SetPBRRandomProperties(box);

    box->setTransLocal(Vec3(x, y, z));
    box->setScale(Vec3(x_l, y_l, z_l));    
    box->RegisterComponent(std::make_unique<ColliderComponent>(box));
    auto col_com = box->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
    col_com->RegisterShape(new BoxShape(x_l * 0.5f, y_l * 0.5f, z_l * 0.5f));
}

static void CreateCapsuleObstacle(float x, float y, float z, float radius, float half_z) {
    using namespace GComponent;
    static int idx = 0;
    static const std::string obj_name = "capsule";
    ObjectManager::getInstance().CreateInstance(obj_name);
    Model* capsule = ModelManager::getInstance().GetModelByName(obj_name + std::to_string(idx++));
	SetPBRRandomProperties(capsule);

    capsule->setTransLocal(Vec3(x, y, z));
    capsule->setScale(Vec3(radius / 0.3, radius / 0.3, (half_z/ 0.7 + radius / 0.3) * 0.5f));
    capsule->RegisterComponent(std::make_unique<ColliderComponent>(capsule));
    auto col_com = capsule->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
    col_com->RegisterShape(new CapsuleShape(radius, half_z));
}

static void SceneInitialize() {
	GComponent::DUAL_ARM_PLATFORM platform;
	CreateCubeObstacle(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	CreateSphereObstacle(0.0f, 0.0f, 0.0f, 1.0f);
	CreateCubeObstacle(0.5f, 0.8f, 1.0f, 0.2f, 0.7f, 0.3f);
	CreateCubeObstacle(-2.5f, -1.8f, 0.0f, 0.4f, 0.3f, 0.5f);
	CreateSphereObstacle(0.35f, 0.45f, 2.5f, 0.8f);

	CreateCubeObstacle(-0.30f, -0.4f, 1.2f, 0.5f, 0.5f, 0.5f);
	CreateCapsuleObstacle(0.55f, -0.5f, 0.0f, 0.1f, 0.3f);
	CreateCapsuleObstacle(0.0f, 0.0f, 0.0f, 0.3f, 0.7f);
	CreateSphereObstacle(0.0f, 0.0f, 0.0f, 1.0f);
	CreateCubeObstacle(0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);
}

static bool scene_initialize = false;

//_____________________________________Test Usage____________________________________________

namespace GComponent {

Viewport::Viewport(QWidget* parent) :
	QOpenGLWidget(parent),
	ui_state_(width(), height()),
	gl_(std::make_shared<GComponent::MyGL>())
{
	qRegisterMetaType<Viewport>("viewport");
	setFocusPolicy(Qt::StrongFocus);
	QSurfaceFormat set_format;
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
	QOpenGLContext::currentContext()->format().setSamples(4);
	gl_->glEnable(GL_MULTISAMPLE);

// Test USage
	if (not scene_initialize) {
		SceneInitialize();
		scene_initialize = true;
	}

// Test Usage
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
	emit EmitDeltaTime(delta_time.count());
	update();
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
	ResourceManager::getInstance().RegisteredShader("skybox",				new MyShader(nullptr,	PathVert(skybox),				PathFrag(skybox)));
	ResourceManager::getInstance().RegisteredShader("postprocess",			new MyShader(nullptr,	PathVert(postprocess),			PathFrag(postprocess)));
	ResourceManager::getInstance().RegisteredShader("no_shadow_color",		new MyShader(nullptr,	PathVert(Color),				PathFrag(Color)));
    ResourceManager::getInstance().RegisteredShader("axis",					new MyShader(nullptr,	PathVert(axis),					PathFrag(axis)));
    ResourceManager::getInstance().RegisteredShader("picking",				new MyShader(nullptr,	PathVert(picking),				PathFrag(picking)));
    ResourceManager::getInstance().RegisteredShader("base",					new MyShader(nullptr,	PathVert(Base),					PathFrag(Base)));
    ResourceManager::getInstance().RegisteredShader("linecolor",			new MyShader(nullptr,	PathVert(LineColor),			PathFrag(LineColor)));
	ResourceManager::getInstance().RegisteredShader("depth_map",			new MyShader(nullptr,	PathVert(depthOrtho),			PathFrag(depthOrtho)));
	ResourceManager::getInstance().RegisteredShader("shadow_color",			new MyShader(nullptr,	PathVert(shadowOrtho),			PathFrag(shadowOrtho)));
	ResourceManager::getInstance().RegisteredShader("csm_depth_map",		new MyShader(nullptr,	PathVert(csm_depth_ortho),		PathFrag(csm_depth_ortho),		PathGeom(csm_depth_ortho)));
	ResourceManager::getInstance().RegisteredShader("color",				new MyShader(nullptr,	PathVert(cascade_shadow_ortho),	PathFrag(cascade_shadow_ortho)));
	// pbr relative
	ResourceManager::getInstance().RegisteredShader("pbr",					new MyShader(nullptr,   PathVert(pbr),					PathFrag(pbr)));		
	ResourceManager::getInstance().RegisteredShader("equirectangular2cube", new MyShader(nullptr,   PathVert(cube),					PathFrag(equirectangular2cubemap)));
	ResourceManager::getInstance().RegisteredShader("irr_conv",				new MyShader(nullptr,   PathVert(skybox),				PathFrag(irradiance_conv)));
	ResourceManager::getInstance().RegisteredShader("pft_conv",				new MyShader(nullptr,   PathVert(skybox),				PathFrag(prefilter_conv)));
	ResourceManager::getInstance().RegisteredShader("brdf_lut",				new MyShader(nullptr,   PathVert(brdf_lut),				PathFrag(brdf_lut)));
}

}