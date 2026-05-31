/**
 *  @file  	picking_helper.h
 *  @brief 	qt version to making picking uv helper class.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Apri 28, 2022
 **/
#ifndef _PICKINGHELPER_H
#define _PICKINGHELPER_H

#include "render/rhi/rhi_device.h"

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
	PickingController(const PickingController& other);
	PickingController& operator=(const PickingController& other);
	~PickingController();

	void SetRhiDevice(const shared_ptr<IRhiDevice>& device);

	bool Init(unsigned width, unsigned height);
	void EnablePickingMode();
	void DisablePickintMode();
	PickingPixelInfo GetPickingPixelInfo(unsigned u, unsigned v);
private:
	void CheckHaveInit();

private:
	bool			 have_init_ = false;
	bool			 owns_framebuffer_ = true;
	shared_ptr<IRhiDevice> rhi_device_;

	RhiFramebufferHandle render_FBO_;
};

}

#endif // !_PICKINGHELPER_H
