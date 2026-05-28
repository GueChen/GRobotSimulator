#include "render/rhi/opengl/opengl_ibl_precompute.h"

#include "render/rhi/opengl/opengl_rhi_device.h"
#include "render/myshader.h"
#include "render/rendermesh.h"
#include "render/uniform_buffer_object.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>

namespace GComponent {

void RunOpenGLIblPrecompute(
	const std::shared_ptr<IRhiDevice>& rhi_device,
	UniformBufferObject& matrices_ubo,
	RenderMesh& sky_box_mesh,
	RenderMesh& quad_mesh,
	MyShader& equirectangular_to_cube_shader,
	MyShader& irradiance_shader,
	MyShader& prefilter_shader,
	MyShader& brdf_lut_shader,
	const std::string& hdr_path)
{
	auto opengl_device = AsOpenGLRhiDevice(rhi_device);
	if (!opengl_device) {
		throw std::runtime_error("IBL precompute currently requires an OpenGL RHI device");
	}
	const auto& gl = opengl_device->GetGL();

	static glm::mat4 capture_proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.0f, 10.0f);
	static glm::mat4 capture_views[] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  -1.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};

	unsigned int hdr_env = gl->LoadTexture(hdr_path, false);
	unsigned int environment_cubemap = 0;
	unsigned int irradiance_cubemap = 0;
	unsigned int prefilter_cubemap = 0;
	unsigned int brdf_lut = 0;

	unsigned fbo = 0;
	unsigned rbo = 0;
	gl->glGenFramebuffers(1, &fbo);
	gl->glGenRenderbuffers(1, &rbo);

	gl->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	gl->glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	gl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	opengl_device->Enable(RhiCapability::DepthTest);
	opengl_device->SetDepthFunc(RhiDepthFunc::LessEqual);
	opengl_device->SetClearColor(RhiClearColor{ 0.0f, 0.0f, 0.0f, 1.0f });

	{
		UBOGaurd ubo_gaurd(&matrices_ubo);
		matrices_ubo.SetSubData(&capture_proj, 0, sizeof glm::mat4);
		int max_texture_size = 0;
		gl->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
		std::cout << max_texture_size << std::endl;
		uint32_t ibl_width = 1024;
		uint32_t ibl_height = 1024;
		gl->glGenTextures(1, &environment_cubemap);
		gl->glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);
		for (uint32_t i = 0; i < 6; ++i) {
			gl->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, ibl_width, ibl_height, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, ibl_width, ibl_height);

		opengl_device->BindTextureUnit(3, RhiTextureHandle{ hdr_env });
		opengl_device->SetViewport(RhiViewport{ 0, 0, static_cast<int>(ibl_width), static_cast<int>(ibl_height) });
		equirectangular_to_cube_shader.use();
		for (uint32_t i = 0; i < 6; ++i) {
			matrices_ubo.SetSubData(&capture_views[i], sizeof glm::mat4, sizeof glm::mat4);
			gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environment_cubemap, 0);
			opengl_device->Clear(RhiClearFlags::Color | RhiClearFlags::Depth);
			sky_box_mesh.Draw();
		}

		uint32_t irr_width = 128;
		uint32_t irr_height = 128;
		gl->glGenTextures(1, &irradiance_cubemap);
		gl->glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_cubemap);
		for (uint32_t i = 0; i < 6; ++i) {
			gl->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, irr_width, irr_height, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irr_width, irr_height);

		opengl_device->BindTextureUnit(3, RhiTextureHandle{ environment_cubemap });
		opengl_device->SetViewport(RhiViewport{ 0, 0, static_cast<int>(irr_width), static_cast<int>(irr_height) });
		irradiance_shader.use();
		for (uint32_t i = 0; i < 6; ++i) {
			matrices_ubo.SetSubData(&capture_views[i], sizeof glm::mat4, sizeof glm::mat4);
			gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_cubemap, 0);
			opengl_device->Clear(RhiClearFlags::Color | RhiClearFlags::Depth);
			sky_box_mesh.Draw();
		}

		uint32_t pft_width = 128;
		uint32_t pft_height = 128;
		gl->glGenTextures(1, &prefilter_cubemap);
		gl->glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_cubemap);
		for (uint32_t i = 0; i < 6; ++i) {
			gl->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, pft_width, pft_height, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		prefilter_shader.use();
		uint32_t max_mipmap_levels = 5;
		for (uint32_t level = 0; level < max_mipmap_levels; ++level) {
			uint32_t mip_width = static_cast<uint32_t>(pft_width * std::pow(0.5, level));
			uint32_t mip_height = static_cast<uint32_t>(pft_height * std::pow(0.5, level));
			gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_width, mip_height);
			opengl_device->SetViewport(RhiViewport{ 0, 0, static_cast<int>(mip_width), static_cast<int>(mip_height) });

			float roughness = static_cast<float>(level) / static_cast<float>(max_mipmap_levels - 1);
			prefilter_shader.setFloat("roughness", roughness);
			for (uint32_t i = 0; i < 6; ++i) {
				matrices_ubo.SetSubData(&capture_views[i], sizeof glm::mat4, sizeof glm::mat4);
				gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_cubemap, level);
				opengl_device->Clear(RhiClearFlags::Color | RhiClearFlags::Depth);
				sky_box_mesh.Draw();
			}
		}

		uint32_t brdf_width = 512;
		uint32_t brdf_height = 512;
		gl->glGenTextures(1, &brdf_lut);
		gl->glBindTexture(GL_TEXTURE_2D, brdf_lut);
		gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdf_width, brdf_height, 0, GL_RG, GL_FLOAT, nullptr);

		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		opengl_device->SetViewport(RhiViewport{ 0, 0, static_cast<int>(brdf_width), static_cast<int>(brdf_height) });
		gl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, brdf_width, brdf_height);
		gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_lut, 0);

		brdf_lut_shader.use();
		opengl_device->Clear(RhiClearFlags::Color | RhiClearFlags::Depth);
		quad_mesh.Draw();
	}

	opengl_device->BindTextureUnit(4, RhiTextureHandle{ irradiance_cubemap });
	opengl_device->BindTextureUnit(5, RhiTextureHandle{ prefilter_cubemap });
	opengl_device->BindTextureUnit(6, RhiTextureHandle{ brdf_lut });
	opengl_device->BindTextureUnit(7, RhiTextureHandle{ environment_cubemap });
	gl->glDeleteFramebuffers(1, &fbo);
	gl->glDeleteRenderbuffers(1, &rbo);
	opengl_device->SetDepthFunc(RhiDepthFunc::Less);
}

} // namespace GComponent
