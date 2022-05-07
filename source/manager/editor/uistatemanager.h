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

enum {
	NoneSelected = 0
};

class UIState
{
public:
	UIState(unsigned, unsigned);
	~UIState()	= default;
	
	void Init(int segments = 15, float radius = 0.045f);
	
	void tick();

	void SetGL(const shared_ptr<MyGL>& gl);

	PickingPixelInfo GetPickingPixelInfo();

	virtual void OnCursorMove(int mouse_pos_x, int mouse_pos_y);
	virtual void OnMousePress(unsigned button_flags);
	virtual void OnMouseReleasse(unsigned button_flags);
	virtual void OnMouseEnter();
	virtual void OnMouseLeave();
	virtual void OnResize(int w, int h);

protected:
	int			  m_mouse_pos_x		= -1;
	int			  m_mouse_pos_y		= -1;
	int			  selected_id		= 0;

	bool		  model_selected    = false;
	bool		  is_draged			= false;
	bool		  is_enter_area		= false;
	bool		  have_init			= false;

	unsigned	  m_width			= 0;
	unsigned	  m_height			= 0;
	unsigned	  button_state		= MouseButton::NoButton;
	
private:
	AxisMode	  m_axis_mode			= AxisMode::Translation;
	AxisSelected  m_axis_selected		= AxisSelected::None;

	QtGLAbstractAxis*		   m_cur_axis			= nullptr;
	QtGLTranslationAxis*	   m_translation_axis;
	QtGLRotationAxis*		   m_rotation_axis;
	QtGLScaleAxis*			   m_scale_axis;

	PickingController		   picking_controller;

	optional<PickingPixelInfo> picking_msg_;

	vec3 scale = vec3(0.2f);
};
}
#endif // !_UISTATE_H



