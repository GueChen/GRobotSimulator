#ifndef GSIM_RENDER_RHI_FACTORY_H
#define GSIM_RENDER_RHI_FACTORY_H

#include "render/rhi/rhi_device.h"

#include <memory>
#include <vector>

namespace GComponent {

std::shared_ptr<IRhiDevice> CreateRhiDevice(const RhiDeviceInitConfig& config = {});
std::shared_ptr<IRhiDevice> CreateOpenGLRhiDevice();
std::shared_ptr<IRhiDevice> CreateOpenGLRhiDevice(const RhiDeviceInitConfig& config);
std::shared_ptr<IRhiDevice> CreateDirectX12RhiDevice();
std::shared_ptr<IRhiDevice> CreateDirectX12RhiDevice(const RhiDeviceInitConfig& config);
bool IsRhiBackendSupported(RhiBackendType backend);
std::vector<RhiBackendType> GetSupportedRhiBackends();

} // namespace GComponent

#endif // GSIM_RENDER_RHI_FACTORY_H
