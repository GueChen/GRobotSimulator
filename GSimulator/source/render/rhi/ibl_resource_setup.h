#ifndef GSIM_RENDER_RHI_IBL_RESOURCE_SETUP_H
#define GSIM_RENDER_RHI_IBL_RESOURCE_SETUP_H

#include "render/rhi/rhi_device.h"

#include <cstdint>
#include <memory>
#include <string>

namespace GComponent {

class MyShader;
class RenderMesh;
class UniformBufferObject;

inline constexpr uint32_t kIblIrradianceBinding = 4;
inline constexpr uint32_t kIblPrefilterBinding = 5;
inline constexpr uint32_t kIblBrdfLutBinding = 6;
inline constexpr uint32_t kIblEnvironmentBinding = 7;

enum class IblSetupStatus {
	Ready,
	FallbackReady,
	Unavailable
};

struct IblSetupContext {
	std::shared_ptr<IRhiDevice> rhi_device;
	UniformBufferObject* matrices_ubo = nullptr;
	RenderMesh* sky_box_mesh = nullptr;
	RenderMesh* quad_mesh = nullptr;
	MyShader* equirectangular_to_cube_shader = nullptr;
	MyShader* irradiance_shader = nullptr;
	MyShader* prefilter_shader = nullptr;
	MyShader* brdf_lut_shader = nullptr;
	std::string hdr_path;
};

struct IblSetupResult {
	IblSetupStatus status = IblSetupStatus::Unavailable;
	bool resources_bound = false;
	std::string reason;

	[[nodiscard]] bool IsUsable() const
	{
		return resources_bound;
	}
};

[[nodiscard]] const char* ToString(IblSetupStatus status);
[[nodiscard]] IblSetupResult SetupImageBasedLightingResources(const IblSetupContext& context);

} // namespace GComponent

#endif // GSIM_RENDER_RHI_IBL_RESOURCE_SETUP_H
