/**
 *  @file  	FrameBufferObject.h
 *  @brief 	Frame Buffer Object to convinient some operation.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	July 5th, 2022
 **/
#ifndef __GFRAME_BUFFER_OBJECT_H
#define __GFRAME_BUFFER_OBJECT_H


#include "render/rhi/rhi_device.h"

#include <memory>

namespace GComponent {

class FrameBufferObject
{
public:
	using type = FrameBufferObject;
	enum AttachType {
		Color, Depth, Cube, CubeMipmap
	};
	struct BufferOption {
		int internal_format;
		int format;
		int min_filter;
		int mag_filter;
		int wrap;
		int attachment;
		int draw_buffer;
		int read_buffer;
	};
public:
	// generate 2D Texture Buffer with a texture
	FrameBufferObject(int width, int height, AttachType type, const std::shared_ptr<IRhiDevice>& rhi_device);
	FrameBufferObject(int width, int height, int level, AttachType type, const std::shared_ptr<IRhiDevice>& rhi_device);
	~FrameBufferObject();

/// fbo bind/relase methods
	void Bind();
	void Release();

/// texture bind/release methods
	void BindTexture(uint32_t texture_pos);
	void ReleaseTexture();

/// setter & getter
	inline unsigned int GetTextureID() const { return static_cast<unsigned int>(texture_buffer_.value); }
	inline void			SetTextureID(unsigned int id) { texture_buffer_ = RhiTextureHandle{ id }; }
	
	[[nodiscard("return texture ID may leak GPU memory")]]
	unsigned int		TakeTexture();

	unsigned int		ReAllocateTexture(int width, int height, AttachType type);

	void				AdjustRenderBufferStorage(int width, int height);

/// copy methods
	FrameBufferObject(const FrameBufferObject& other)				= delete;
	FrameBufferObject& operator=(const FrameBufferObject& other)	= delete;

/// move methods
	FrameBufferObject(FrameBufferObject&& other)			noexcept;
	FrameBufferObject& operator=(FrameBufferObject&& other) noexcept;
private:
	void Clear();
	void Initialize(int width, int height, int levels, AttachType type);
	static RhiFramebufferAttachment ToRhiAttachment(AttachType type);

private:	
	RhiFramebufferHandle			frame_buffer_;
	RhiTextureHandle				texture_buffer_;
	uint32_t						texture_pos_    = 0;
	std::shared_ptr<IRhiDevice>		rhi_device_		= nullptr;
};

class FBOGuard {
public:
	FBOGuard(FrameBufferObject* fbo) :fbo_ptr(fbo) {
		if (fbo_ptr) fbo_ptr->Bind();
	}
	~FBOGuard() {
		if (fbo_ptr) fbo_ptr->Release();
	}
private:
	FrameBufferObject* fbo_ptr = nullptr;
};

class FBOTextureGuard {
public:
	FBOTextureGuard(FrameBufferObject* fbo, uint32_t texture_pos = 0) :fbo_ptr(fbo) {
		if (fbo_ptr) fbo_ptr->BindTexture(texture_pos);
	}
	~FBOTextureGuard() {
		if (fbo_ptr) fbo_ptr->ReleaseTexture();
	}
private:
	FrameBufferObject* fbo_ptr = nullptr;
};

}	// !namespace GComponent
#endif // !__FRAME_BUFFER_OBJECT_H
