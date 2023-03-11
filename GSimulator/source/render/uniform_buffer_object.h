/**
 *  @file  	uniform_buffer_object.h
 *  @brief 	uniform buffer object for convinience about data operations.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 11th, 2023
 **/
#ifndef __UNIFORM_BUFFER_OBJECT
#define __UNIFORM_BUFFER_OBJECT

#include "render/mygl.hpp"

#include <memory>

namespace GComponent {

class UniformBufferObject {
public:
	UniformBufferObject(int binding, size_t size, const std::shared_ptr<MyGL>& other);
	~UniformBufferObject();

	inline void Bind()			{ gl_->glBindBuffer(GL_UNIFORM_BUFFER, ubo_); }
	inline void Release()		{ gl_->glBindBuffer(GL_UNIFORM_BUFFER, 0); }

	void SetData   (const void* data, size_t size);
	void SetSubData(const void* data, size_t offset, size_t size);

	UniformBufferObject(const UniformBufferObject& other)			 = delete;
	UniformBufferObject& operator=(const UniformBufferObject& other) = delete;

	UniformBufferObject(UniformBufferObject&& other)				 noexcept;
	UniformBufferObject& operator=(UniformBufferObject&& other)		 noexcept;

private:
	void Clear();

private:
	unsigned int		  ubo_		   = 0;
	int					  binding_pos_ = -1;
	size_t				  size_	       = 0;
	std::shared_ptr<MyGL> gl_		   = nullptr;

};

class UBOGaurd {
public:
	UBOGaurd(UniformBufferObject* ubo) : ptr(ubo)	{
		if (ptr) ptr->Bind();
	}
	~UBOGaurd() {
		if (ptr) ptr->Release();
	}
private:
	UniformBufferObject* ptr = nullptr;
};

}


#endif