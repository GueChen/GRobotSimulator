/**
 *  @file  	FrameBufferObject.h
 *  @brief 	Frame Buffer Object to convinient some operation.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	July 5th, 2022
 **/
#ifndef __GFRAME_BUFFER_OBJECT_H
#define __GFRAME_BUFFER_OBJECT_H


#include "render/mygl.hpp"

#include <QtGui/QOpenGLContext>

#include <memory>

namespace GComponent {

class FrameBufferObject
{
public:
	using type = FrameBufferObject;

public:
	FrameBufferObject(int width, int height, const std::shared_ptr<MyGL>& other);
	~FrameBufferObject();

/// fbo bind/relase methods
	inline void Bind()				{ gl_->glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_); }
	inline void Release()			{ gl_->glBindFramebuffer(GL_FRAMEBUFFER, GetDefaultFBO()); }

/// texture bind/release methods
	inline void BindTexture()		{ gl_->glBindTexture(GL_TEXTURE_2D, texture_buffer_); }
	inline void ReleaseTexture()	{ gl_->glBindTexture(GL_TEXTURE_2D, 0); }

/// copy methods  拷贝函数
	FrameBufferObject(const FrameBufferObject& other)				= delete;
	FrameBufferObject& operator=(const FrameBufferObject& other)	= delete;

/// move methods  移动函数
	FrameBufferObject(FrameBufferObject&& other);
	FrameBufferObject& operator=(FrameBufferObject&& other);
private:
	void Clear();
/// static methods 静态方法
	inline static unsigned GetDefaultFBO() { return QOpenGLContext::currentContext()->defaultFramebufferObject(); };
private:	
	unsigned						frame_buffer_	= 0;
	unsigned						texture_buffer_ = 0;
	unsigned						render_buffer_	= 0;
	std::shared_ptr<MyGL>			gl_				= nullptr;
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
	FBOTextureGuard(FrameBufferObject* fbo) :fbo_ptr(fbo) {
		if (fbo_ptr) fbo_ptr->BindTexture();
	}
	~FBOTextureGuard() {
		if (fbo_ptr) fbo_ptr->ReleaseTexture();
	}
private:
	FrameBufferObject* fbo_ptr = nullptr;
};

}	// !namespace GComponent
#endif // !__FRAME_BUFFER_OBJECT_H
