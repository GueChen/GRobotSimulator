/**
 *  @file  	ui_state.h
 *  @brief 	This class Define all state Pass through UI.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 6th, 2022
 **/
#ifndef _UISTATE_H
#define _UISTATE_H

#include "render/mygl.hpp"
#include "model/axis/qtaxis.h"
#include "function/picking_helper.h"

#include <glm/glm.hpp>

#include <QtCore/QObject>

#include <mutex>
#include <memory>
#include <optional>

namespace GComponent {

using std::mutex;
using std::optional;
using std::shared_ptr;

enum MouseButton {
	NoButton		= 0x00000000,
	LeftButton		= 0x00000001,
	RightButton		= 0x00000002,
	MiddleButton	= 0x00000004
};

enum class KeyButtonState: size_t 
{
	None			= 0x0,
	KeyW			= 0x1 << 0,
	KeyS			= 0x1 << 1,
	KeyA			= 0x1 << 2,
	KeyD			= 0x1 << 3,
	KeyDelete		= 0x1 << 30
};

enum {
	NoneSelected	 = 0,
	BufferNoValue	 = -1
};

class UIState : public QObject	
{
	Q_OBJECT
public:
	UIState(unsigned w, unsigned h, int segments = 15, float radius = 0.045f);
	~UIState();
		
	void tick();
	void SetGL(const shared_ptr<MyGL>& gl);
	PickingPixelInfo GetPickingPixelInfo();
	Model* GetSelectedObject() const;

	virtual void OnCursorMove(int mouse_pos_x, int mouse_pos_y);
	virtual void OnMousePress(unsigned button_flags);
	virtual void OnMouseRelease(unsigned button_flags);
	virtual void OnMouseEnter(int mouse_pos_x, int mouse_pos_y);
	virtual void OnMouseLeave();
	virtual void OnKeyPress(size_t key_state);
	virtual void OnKeyRelease(size_t key_state);
	virtual void OnResize(int w, int h);

private:
	void Init(int segments = 15, float radius = 0.045f);
	void ProcessDelete();

signals:
	void DeleteRequest(const string& msg);
	void SelectRequest(const string& msg);

public slots:
	void ResponseSelectRequest(const string& select_obj_name);
	void ResponseAxisModeChange(AxisMode mode);

protected:
	int			  m_mouse_pos_x			= -1;
	int			  m_mouse_pos_y			= -1;
	int			  m_last_mouse_pos_x	= -1;
	int			  m_last_mouse_pos_y    = -1;
	int			  m_mouse_delta_x		= 0;
	int			  m_mouse_delta_y       = 0;
	int			  selected_id			= NoneSelected;
	int			  selected_id_buffer	= BufferNoValue;

	bool		  is_draged				= false;
	bool		  is_enter_area			= false;
	bool		  is_init				= false;

	float		  m_aspect				= 0.0f;
	unsigned	  m_width				= 0;
	unsigned	  m_height				= 0;	
	unsigned	  button_state			= MouseButton::NoButton;
	size_t		  m_key_state			= static_cast<size_t>(KeyButtonState::None);
	
private:
	AxisMode				m_axis_mode			    = AxisMode::Translation;
	AxisSelected			   m_axis_selected		= AxisSelected::None;

	QtGLAbstractAxis*		   m_cur_axis			= nullptr;
	QtGLTranslationAxis*	   m_translation_axis	= nullptr;
	QtGLRotationAxis*		   m_rotation_axis		= nullptr;
	QtGLScaleAxis*			   m_scale_axis			= nullptr;

	optional<PickingPixelInfo> picking_msg_ = std::nullopt;

	PickingController		   picking_controller;
	
};
}
#endif // !_UISTATE_H



