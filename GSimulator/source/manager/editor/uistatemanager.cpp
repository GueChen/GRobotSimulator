/**
 *  @file  	ui_state.cpp
 *  @brief 	This class process all state Pass through UI, this file is implementations.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 7th, 2022
 **/
#include "manager/editor/uistatemanager.h"

#include "manager/rendermanager.h"
#include "manager/modelmanager.h"

#include <exception>
#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // !

GComponent::UIState::UIState(unsigned w, unsigned h, int segments, float radius):
	m_width(w), m_height(h), m_aspect(static_cast<float>(m_width)/ m_height)
{
	m_translation_axis	= new QtGLTranslationAxis;
	m_scale_axis		= new QtGLScaleAxis;
	m_rotation_axis		= new QtGLRotationAxis;

	ModelManager::getInstance().RegisteredAuxiModel(m_translation_axis->getName(), m_translation_axis);
	ModelManager::getInstance().RegisteredAuxiModel(m_rotation_axis->getName(),	   m_rotation_axis);
	ModelManager::getInstance().RegisteredAuxiModel(m_scale_axis->getName(),       m_scale_axis);

	Init(segments, radius);

	RenderManager::getInstance().m_render_sharing_msg.SetViewportSize(m_width, m_height);
}

GComponent::UIState::~UIState()
{
	m_cur_axis = nullptr;
}

void GComponent::UIState::tick()
{
	ProcessDelete();

	if (is_enter_area) 
	{
		// picking process
		picking_msg_ = GetPickingPixelInfo();
		RenderManager::getInstance().SetPickingController(picking_controller);

		// mouse pos process
		m_mouse_delta_x		= m_mouse_pos_x - m_last_mouse_pos_x;
		m_mouse_delta_y		= m_mouse_pos_y - m_last_mouse_pos_y;
		m_last_mouse_pos_x	= m_mouse_pos_x;
		m_last_mouse_pos_y	= m_mouse_pos_y;
	}
	else 
	{
		// picking process
		if (selected_id_buffer != BufferNoValue) 
		{
			selected_id		   = selected_id_buffer;
			selected_id_buffer = BufferNoValue;
		}
		picking_msg_ = std::nullopt;

		// mouse pos process
		m_mouse_pos_x	= m_last_mouse_pos_x = -1;
		m_mouse_pos_y	= m_last_mouse_pos_y = -1;
		m_mouse_delta_x = 0;
		m_mouse_delta_y = 0;
	}

	if (selected_id != NoneSelected) 
	{		
		switch (m_axis_mode) 
		{
			using enum AxisMode;
			case Translation:	m_cur_axis = m_translation_axis;break;
			case Rotation:		m_cur_axis = m_rotation_axis;	break;
			case Scale:			m_cur_axis = m_scale_axis;		break;	
			default:			m_cur_axis = nullptr;
		}
	}

	if (is_draged && m_axis_selected != AxisSelected::None) {
		Camera* camera = ModelManager::getInstance().GetCameraByHandle(1);		
		Mat4 inv_proj = InverseSE3(Conversion::toMat4f(camera->GetProjection(m_aspect)));	
		Mat3 inv_view = InverseSE3(Conversion::toMat4f(camera->GetViewMatrix())).block(0, 0, 3, 3);
		Vec4 screen_uv_coor(static_cast<float>(m_mouse_delta_x) / m_width,
							static_cast<float>(-m_mouse_delta_y) / m_height, 
							0.0f, 1.0f);

		Vec3 line_in_eye   =  Vec3::Zero();
		line_in_eye.block(0, 0, 2, 1) = (inv_proj * screen_uv_coor).block(0, 0, 2, 1);
		Vec3 line_in_world = inv_view * line_in_eye;

		Vec3 dir;
		switch (m_axis_selected) {
			using enum AxisSelected;
			case xAxis:	dir = Vec3(1.0f, 0.0f, 0.0f); break;
			case yAxis: dir = Vec3(0.0f, 1.0f, 0.0f); break;
			case zAxis: dir = Vec3(0.0f, 0.0f, 1.0f); break;
		}	
		dir = dir.dot(line_in_world) * dir;
		if (Model* selected_obj = ModelManager::getInstance().GetModelByHandle(selected_id);
			selected_obj) {
			switch (m_axis_mode) { using enum AxisMode;
			case Translation: {
				Eigen::Matrix4f global_trans    = selected_obj->getTranslationModelMatrix();
				global_trans.block(0, 3, 3, 1) += dir;
				Eigen::Matrix4f local_trans     = InverseSE3(selected_obj->getParentModelMatrix()) * global_trans;			
				selected_obj->setTransLocal(local_trans.block(0, 3, 3, 1));
				break;
			}
			case Rotation: {			
				Eigen::Matrix3f global_rot_dcm  = selected_obj->getModelMatrixWithoutScale().block(0, 0, 3, 3);				
				Eigen::Matrix3f local_rot_dcm   = selected_obj->getParentModelMatrix().block(0, 0, 3, 3).inverse() * Roderigues(dir) * global_rot_dcm;			
				selected_obj->setRotLocal(LogMapSO3Toso3(local_rot_dcm));
				break;
			}
			case Scale: {				
				auto scale = selected_obj->getScale();
				scale += dir;
				selected_obj->setScale(scale);
			}
			}
		}

	}
	else if (picking_msg_ && picking_msg_->drawID == static_cast<float>(PassType::AuxiliaryPass) && m_cur_axis) {
		std::cout << std::format("picking info: Stri -- {:<8}, Prim -- {:<8}, Draw -- {:}, Model -- {:}\n", m_cur_axis->GetStridedSize(), picking_msg_->primitiveID,
			picking_msg_->drawID, picking_msg_->modelID);
		if (picking_msg_->primitiveID <= m_cur_axis->GetStridedSize())
		{
			m_axis_selected = AxisSelected::xAxis;
		}
		else if (picking_msg_->primitiveID <= 2 * m_cur_axis->GetStridedSize())
		{
			m_axis_selected = AxisSelected::yAxis;
		}
		else
		{
			m_axis_selected = AxisSelected::zAxis;
		}
	}
	else{
			m_axis_selected = AxisSelected::None;
	}
	
	if (Model* selected_obj = ModelManager::getInstance().GetModelByHandle(selected_id);
		selected_obj)
	{
		RenderManager::getInstance().m_selected_id = selected_id;

		Camera* camera = ModelManager::getInstance().GetCameraByHandle(1);
		Mat4 view	   = Conversion::toMat4f(camera->GetViewMatrix());
		Vec4 glb_pos   = Vec4::Ones();
		glb_pos.block(0, 0, 3, 1) 
					   = selected_obj->getModelMatrix().block(0, 3, 3, 1);
		
		float distance = ((InverseSE3(view) * glb_pos).block(0, 0, 3, 1)).norm();

		Mat4 scale_mat = Mat4::Identity();
		scale_mat.block(0, 0, 3, 3) = Scale((Vec3::Ones() * distance * 0.05f).eval());
		
		if (m_cur_axis) {
			m_cur_axis->SetAxisSelected(m_axis_selected);
			m_cur_axis->setModelMatrix(
				(m_axis_mode != AxisMode::Scale ? selected_obj->getTranslationModelMatrix() : selected_obj->getModelMatrixWithoutScale())
				* scale_mat);
			m_cur_axis->tick(0.0f);
		}
	}

}

void GComponent::UIState::SetGL(const shared_ptr<MyGL>& gl)
{
	picking_controller.SetGL(gl);
	picking_controller.Init(m_width, m_height);	
	
}

GComponent::PickingPixelInfo GComponent::UIState::GetPickingPixelInfo()
{
	return picking_controller.GetPickingPixelInfo(m_mouse_pos_x, m_height - m_mouse_pos_y - 1);
}

GComponent::Model* GComponent::UIState::GetSelectedObject() const
{
	return ModelManager::getInstance().GetModelByHandle(selected_id);
}

/*__________________________________PRIVATE METHODS_____________________________________________*/
void GComponent::UIState::ProcessDelete()
{
	static bool has_response_delete = false;
	if (!has_response_delete)
	{
		if (m_key_state & static_cast<size_t>(KeyButtonState::KeyDelete) && selected_id != NoneSelected)
		{
			has_response_delete = true;
			emit DeleteRequest(ModelManager::getInstance().GetNameByID(selected_id));
			ModelManager::getInstance().DeregisteredModel(selected_id);
			selected_id = NoneSelected;
		}
	}
	else
	{
		if (~m_key_state & static_cast<size_t>(KeyButtonState::KeyDelete))
		{
			has_response_delete = false;
		}
	}
}

void GComponent::UIState::Init(int segments, float radius)
{
	if (!is_init) {
		m_translation_axis->Init(segments, radius);
		m_scale_axis->Init(segments, radius);
		m_rotation_axis->Init(8 * segments, 15.0f * radius);
	}
	is_init = true;
}

/*____________________________________UI PROCESS_________________________________________________*/
void GComponent::UIState::OnCursorMove(int mouse_pos_x, int mouse_pos_y)
{
	m_mouse_pos_x = mouse_pos_x;
	m_mouse_pos_y = mouse_pos_y;
}

void GComponent::UIState::OnMousePress(unsigned button_flag)
{
	button_state = button_flag;
	if (button_state & MouseButton::LeftButton) 
	{
		if (picking_msg_) 
		{
			if (picking_msg_->drawID == static_cast<float>(PassType::DirLightPass)) 
			{
				selected_id = picking_msg_->modelID;			
			}
			else if (picking_msg_->drawID == static_cast<float>(PassType::AuxiliaryPass)) 
			{
				is_draged	= true;				
			}
			else
			{
				selected_id = 0;
			}
			emit SelectRequest(ModelManager::getInstance().GetNameByID(selected_id));
		}			
	}
	if (button_state & MouseButton::RightButton) 
	{

	}
	if (button_state & MouseButton::MiddleButton)
	{

	}
}

void GComponent::UIState::OnMouseRelease(unsigned button_flag)
{
	button_state &= (~button_flag);
	if (!(button_state & MouseButton::LeftButton))
	{
		is_draged = false;		
	}
}

void GComponent::UIState::OnMouseEnter(int mouse_pos_x, int mouse_pos_y)
{
	is_enter_area = true;
	m_mouse_pos_x = m_last_mouse_pos_x = mouse_pos_x;
	m_mouse_pos_y = m_last_mouse_pos_y = mouse_pos_y;
}

void GComponent::UIState::OnMouseLeave()
{
	is_enter_area = false;
}

void GComponent::UIState::OnKeyPress(size_t key_state)
{
	m_key_state |= key_state;
}

void GComponent::UIState::OnKeyRelease(size_t key_state)
{
	m_key_state &= ~key_state;
}

void GComponent::UIState::OnResize(int w, int h)
{
	m_width  = w, m_height = h;
	m_aspect = static_cast<float>(w) / h;
	picking_controller.Init(m_width, m_height);

	RenderManager& render_manager = RenderManager::getInstance();
	render_manager.m_render_sharing_msg.SetViewportSize(w, h);
	render_manager.InitFrameBuffer();
}

void GComponent::UIState::OnWheel(int val)
{
}

/*_____________________________________SLOT FUNCTIONS________________________________________________*/
void GComponent::UIState::ResponseSelectRequest(const string& select_obj_name) 
{
	selected_id_buffer = ModelManager::getInstance().GetIDByName(select_obj_name);
}

void GComponent::UIState::ResponseAxisModeChange(AxisMode mode)
{
	m_axis_mode = mode;
}
