#ifndef GSIM_RENDER_RHI_COMMAND_H
#define GSIM_RENDER_RHI_COMMAND_H

#include "render/rhi/rhi_resource.h"

#include <string>

namespace GComponent {

struct RhiRenderPassDesc {
	std::string name;
	RhiFramebufferHandle framebuffer;
	RhiViewport viewport;
	RhiClearFlags clear_flags = RhiClearFlags::None;
	RhiClearColor clear_color;
};

struct RhiDrawCommand {
	std::string object_name;
	std::string mesh_name;
	std::string shader_name;
	RhiMeshHandle mesh;
	RhiShaderHandle shader;
	RhiPrimitiveTopology topology = RhiPrimitiveTopology::Triangles;
};

} // namespace GComponent

#endif // GSIM_RENDER_RHI_COMMAND_H
