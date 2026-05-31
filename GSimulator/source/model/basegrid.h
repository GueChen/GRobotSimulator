#ifndef BASEGRID_H
#define BASEGRID_H

#include "render/rendering_datastructure.hpp"
#include "render/rhi/rhi_device.h"

#include <glm/glm.hpp>

#include <memory>
#include <array>

namespace GComponent {


using std::shared_ptr;
using vec3 = glm::vec3;

// TODO: Improve this object with a better construction pattern.
// TODO: Extract common behavior into a base type.

class BaseGrid
{
private:
    float gridSize;

    /* resource handles */
    RhiMeshHandle mesh_;
    shared_ptr<IRhiDevice> rhi_device_;
    uint32_t vertex_count_ = 0;

    /* initialization flag */
    bool isInit = false;

public:
    /* construction and destruction */
    BaseGrid(int n, float size = 0.05f);
    ~BaseGrid();

    /* RHI setup */
    void SetRhiDevice(shared_ptr<IRhiDevice> rhi_device);

    /* draw interface */
    void Draw();

private:
    /* initialization */
    void RhiBufferInitialize();

    static std::array<vec3, 6> GetFullscreenQuadVertexLocation();
};


}


#endif // BASEGRID_H
