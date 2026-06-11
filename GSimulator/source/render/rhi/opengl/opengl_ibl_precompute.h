#ifndef GSIM_RENDER_RHI_OPENGL_IBL_PRECOMPUTE_H
#define GSIM_RENDER_RHI_OPENGL_IBL_PRECOMPUTE_H

#include "render/rhi/ibl_resource_setup.h"

#include <string_view>

namespace GComponent {

[[nodiscard]] IblSetupResult RunOpenGLIblPrecompute(const IblSetupContext& context);
[[nodiscard]] IblSetupResult BindOpenGLFallbackIblResources(const IblSetupContext& context, std::string_view reason);

} // namespace GComponent

#endif // GSIM_RENDER_RHI_OPENGL_IBL_PRECOMPUTE_H
