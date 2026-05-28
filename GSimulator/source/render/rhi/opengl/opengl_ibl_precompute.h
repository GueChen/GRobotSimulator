#ifndef GSIM_RENDER_RHI_OPENGL_IBL_PRECOMPUTE_H
#define GSIM_RENDER_RHI_OPENGL_IBL_PRECOMPUTE_H

#include "render/rhi/rhi_device.h"

#include <memory>
#include <string>

namespace GComponent {

class MyShader;
class RenderMesh;
class UniformBufferObject;

void RunOpenGLIblPrecompute(
	const std::shared_ptr<IRhiDevice>& rhi_device,
	UniformBufferObject& matrices_ubo,
	RenderMesh& sky_box_mesh,
	RenderMesh& quad_mesh,
	MyShader& equirectangular_to_cube_shader,
	MyShader& irradiance_shader,
	MyShader& prefilter_shader,
	MyShader& brdf_lut_shader,
	const std::string& hdr_path);

} // namespace GComponent

#endif // GSIM_RENDER_RHI_OPENGL_IBL_PRECOMPUTE_H
