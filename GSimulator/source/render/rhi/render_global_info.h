/**
 *  @file  	render_global_info.h
 *  @brief 	some sharing msg used in render.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	July 12th, 2022
 **/
#ifndef __RENDER_RHI_H
#define __RENDER_RHI_H

#include "render/camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace GComponent {

struct RenderViewport {
	glm::vec2 window_pos	= glm::vec2(0.0f);
	glm::vec2 window_size	= glm::vec2(0.0f);
};

struct SimpleDirLight {
	glm::vec3 dir		= glm::vec3(0.0f);
	glm::vec3 color		= glm::vec3(0.0f);

	void Rotate(float radians, glm::vec3 axis) {
		dir = glm::rotate(glm::mat4(1.0f), radians, axis) * glm::vec4(dir, 1.0f);
	}
};

struct ProjectionInfo {
	float aspect			= 0.0f;
	float fov				= 0.0f;
	float near_plane		= 0.0f;
	float far_plane			= 0.0f;

	glm::mat4 GetMatrix() {
		return glm::perspective(glm::radians(fov), aspect, near_plane, far_plane);
	}
};

struct RenderGlobalInfo
{
/*_________________DATA FIELDS_____________________________________________________________________*/
	RenderViewport		viewport;							
	SimpleDirLight		dir_light;							
	ProjectionInfo		projection_info;					
		
	glm::vec3			view_pos		= glm::vec3(0.0f);
	glm::mat4			view_mat		= glm::mat4(1.0f);
	glm::mat4			projection_mat	= glm::mat4(1.0f);
	
	const glm::vec3		GlobalUp		= glm::vec3(0.0f, 0.0f, 1.0f);

/*_________________Setter function make User code cleaner__________________________________________*/
	void SetProjectionPlane(float near_plane, float far_plane) {
		projection_info.near_plane = near_plane;
		projection_info.far_plane  = far_plane;
	}

	void SetViewportSize(int w, int h) {
		viewport.window_size.x = w;
		viewport.window_size.y = h;
		projection_info.aspect = static_cast<float>(w) / h;
	}

	void SetCameraInfo(const Camera& camera) {
		projection_info.fov = camera.Zoom;		
		view_pos			= camera.Position;
		view_mat			= camera.GetViewMatrix();
	}

	void SetSimpleDirLight(const glm::vec3& dir, const glm::vec3& color) {
		dir_light.dir   = dir;
		dir_light.color = color;
	}

	void UpdateProjectionMatrix() {
		projection_mat = projection_info.GetMatrix();
	}
};

}	// !namespace GComponent



#endif // !__RENDER_RHI_H





