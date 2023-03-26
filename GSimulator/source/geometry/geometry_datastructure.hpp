#ifndef __GEOMETRY_DATASTRUCTURE
#define __GEOMETRY_DATASTRUCTURE

#include "render/rendering_datastructure.hpp"

#include <Eigen/Dense>

#include <numeric>

namespace GComponent {

struct RawBoundingBox {
	Eigen::Vector3<float>  m_min = Eigen::Vector3<float>::Ones() * std::numeric_limits<float>::max();
	Eigen::Vector3<float>  m_max = Eigen::Vector3<float>::Ones() * std::numeric_limits<float>::lowest();
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
