#ifndef __GEOMETRY_DATASTRUCTURE
#define __GEOMETRY_DATASTRUCTURE

#include "render/rendering_datastructure.hpp"

#include <Eigen/Dense>

namespace GComponent {

struct RawBoundingBox {
	Eigen::Vector3<float>  m_min;
	Eigen::Vector3<float>  m_max;
};

struct RawConvex {
	std::vector<Vertex>   m_vertices;
	std::vector<Triangle> m_triangles;

	float				  m_volume;
	Eigen::Vector3<float> m_center;
	RawBoundingBox        m_bounding_box;
};
}

#endif //!__GEOMETRY_DATASTRUCTURE
