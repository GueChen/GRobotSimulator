/**
 *  @file  	picking_helper.h
 *  @brief 	qt version to making picking uv helper class.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Apri 28, 2022
 **/
#ifndef _PICKINGHELPER_H
#define _PICKINGHELPER_H

#include "render/mygl.hpp"

#include <memory>

namespace GComponent {

class PickingController;

using std::shared_ptr;
/*
* @brief 图片索引的 id
* 默认 r 代表绘制 Pass 的 ID, g 代表 model 对象的 ID, b 代表图元 ID
* */
struct PickingPixelInfo {
	float drawID		= 0.0f;		
	float modelID		= 0.0f;		
	float primitiveID	= 0.0f;	
};

class PickingGuard {
public:
	PickingGuard(PickingController & controller);
	~PickingGuard();

	PickingGuard(const PickingGuard& other)				= delete;
	PickingGuard& operator=(const PickingGuard& other)	= delete;

private:
	PickingController& controller_;
};

class PickingController
{
public:
	PickingController();
	~PickingController();

	void SetGL(const shared_ptr<MyGL>& other);

	bool Init(unsigned width, unsigned height);
	void EnablePickingMode(unsigned default_FBO);
	void DisablePickintMode();
	PickingPixelInfo GetPickingPixelInfo(unsigned u, unsigned v);
private:
	void CheckHaveInit();

private:
	bool			 have_init_;
	shared_ptr<MyGL> gl_;

	unsigned default_FBO_			   = 0;
	unsigned render_FBO_					   = 0;
	unsigned picking_texture_obeject_  = 0;
	unsigned depth_texture_object_	   = 0;
};

}

#endif // !_PICKINGHELPER_H



