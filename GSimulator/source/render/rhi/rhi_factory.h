#ifndef GSIM_RENDER_RHI_FACTORY_H
#define GSIM_RENDER_RHI_FACTORY_H

#include "render/rhi/rhi_device.h"

#include <memory>

namespace GComponent {

std::shared_ptr<IRhiDevice> CreateOpenGLRhiDevice();

} // namespace GComponent

#endif // GSIM_RENDER_RHI_FACTORY_H
