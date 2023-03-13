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
#include <map>

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
	FrameBufferObject(int width, int height, AttachType type, const std::shared_ptr<MyGL>& other);
	FrameBufferObject(int width, int height, int level, AttachType type, const std::shared_ptr<MyGL>& other);
	~FrameBufferObject();

/// fbo bind/relase methods
	inline void Bind()				{ gl_->glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_); }
	inline void Release()			{ gl_->glBindFramebuffer(GL_FRAMEBUFFER, GetDefaultFBO()); }

/// texture bind/release methods
	inline void BindTexture(int texture_pos)		
									{ 
										texture_pos_ = texture_pos;								
										gl_->glActiveTexture(texture_pos_);
										gl_->glBindTexture(texture_type_, texture_buffer_); 
									}
	inline void ReleaseTexture()	{ 
										gl_->glActiveTexture(texture_pos_);
										gl_->glBindTexture(texture_type_, 0);
									}

/// setter & getter
	inline unsigned int GetTextureID() const { return texture_buffer_; }
	inline void			SetTextureID(unsigned int id) { texture_buffer_ = id; }
	
	[[nodiscard("return texture ID may leak GPU memory")]]
	unsigned int		TakeTexture();

	unsigned int		ReAllocateTexture(int width, int height, AttachType type);

	void				AdjustRenderBufferStorage(int width, int height);

/// copy methods  拷贝函数
	FrameBufferObject(const FrameBufferObject& other)				= delete;
	FrameBufferObject& operator=(const FrameBufferObject& other)	= delete;

/// move methods  移动函数
	FrameBufferObject(FrameBufferObject&& other)			noexcept;
	FrameBufferObject& operator=(FrameBufferObject&& other) noexcept;
private:
	void Clear();
	void Initialize(int width, int height, int levels, AttachType type);
	void GenTexture(int width, int height, int levels, const BufferOption& opt);
	void GenRenderBuffer(int width, int height);
	void BindTextureOnFrameBuffer(const BufferOption& opt);
	

/// static methods 静态方法
	inline static unsigned GetDefaultFBO() { return QOpenGLContext::currentContext()->defaultFramebufferObject(); };

private:	
	unsigned						frame_buffer_	= 0;
	unsigned						texture_buffer_ = 0;
	unsigned						render_buffer_	= 0;
	unsigned						texture_pos_    = GL_TEXTURE0;
	unsigned						texture_type_   = GL_TEXTURE_2D;
	std::shared_ptr<MyGL>			gl_				= nullptr;

/// static fileds
private:
	static std::map<AttachType, BufferOption> option_map;	
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
	FBOTextureGuard(FrameBufferObject* fbo, int texture_pos = GL_TEXTURE0) :fbo_ptr(fbo) {
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
