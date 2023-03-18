#include "geometry/convex_generate.h"

#define ENABLE_VHACD_IMPLEMENTATION 1
#include <VHACD.h>

#include <cinttypes>
#include <memory>

#include <iostream>
#include <format>

namespace GComponent{
namespace VHACD_Util{
static inline Eigen::Vector3f ToVec3(const VHACD::Vect3& vec) {
	return Eigen::Vector3f(vec.GetX(), vec.GetY(), vec.GetZ());
}


class ProgressLogger : public VHACD::IVHACD::IUserCallback {	
virtual void Update(const double overallProgress, 
					const double stageProgress, 
					const char* const stage, 
					const char* operation) override {
	
	std::cout << std::format(
		"VHACD caculating operation <{:<20}>: {:<20} -- [{:<.5}] | overall -- [{:<.5}]\r",
		operation, stage, stageProgress, overallProgress
	);
}
};

}

using namespace VHACD_Util;

std::vector<RawConvex> GComponent::GenerateConvexHull(const std::vector<Vertex>& vertices,
												  const std::vector<Triangle>& triangles,
												  uint32_t max_convex_hulls, 
												  uint32_t max_vertices_per_ch, 
												  bool use_update_callback)
{
	auto VHACD_Deleter = [](VHACD::IVHACD* ptr) { ptr->Release(); };
	std::unique_ptr<VHACD::IVHACD, decltype(VHACD_Deleter)> vhacd(VHACD::CreateVHACD(), VHACD_Deleter);
	std::unique_ptr<VHACD::IVHACD::IUserCallback> cbk = nullptr;
	VHACD::IVHACD::Parameters params;
	params.m_maxConvexHulls		 = max_convex_hulls;
	params.m_maxNumVerticesPerCH = max_vertices_per_ch;
	
	if (use_update_callback) {
		cbk = std::unique_ptr<VHACD::IVHACD::IUserCallback>(new ProgressLogger);
		params.m_callback = cbk.get();
	}

	std::vector<glm::vec3>  positions; positions.reserve(vertices.size());
	for (auto& vert : vertices) positions.push_back(vert.position);

	bool sucess = vhacd->Compute((float*)positions.data(),	  positions.size(),		// vertices  input
								 (uint32_t*)triangles.data(), triangles.size(),		// triangles input
								 params);

	std::vector<RawConvex> ret;
	if (sucess) {
		uint32_t hull_nb = vhacd->GetNConvexHulls();
		for (int i = 0; i < hull_nb; ++i) {
			RawConvex output_hull;
			VHACD::IVHACD::ConvexHull convex_hull;
			vhacd->GetConvexHull(i, convex_hull);
			
			// tranfer point positions to vertex position attribute
			for (auto& point : convex_hull.m_points) {				
				Vertex vert;
				vert.position = glm::vec3(point.mX, point.mY, point.mZ);
				vert.normal   = glm::zero<glm::vec3>();
				vert.texcoords= glm::zero<glm::vec2>();
				output_hull.m_vertices.push_back(vert);
			}

			// transfer tri index to triangle attribute
			for (auto& tri : convex_hull.m_triangles) {
				Triangle triangle(tri.mI0, tri.mI1, tri.mI2);
				output_hull.m_triangles.push_back(triangle);

				// caculate normal for vertex
				Vertex& a = output_hull.m_vertices[tri.mI0],
					  & b = output_hull.m_vertices[tri.mI1],
					  & c = output_hull.m_vertices[tri.mI2];
				glm::vec3 ac = c.position - a.position,
						  ab = b.position - a.position;
				glm::vec3 normal = glm::cross(ab, ac);
				a.normal += normal;
				b.normal += normal;
				c.normal += normal;
			}

			// normalization for all vertices in convexhull
			for (auto& vert : output_hull.m_vertices) {
				vert.normal = glm::normalize(vert.normal);
			}

			// transfer other attribute
			output_hull.m_center = ToVec3(convex_hull.m_center);
			output_hull.m_volume = convex_hull.m_volume;
			output_hull.m_bounding_box.m_min = ToVec3(convex_hull.mBmin);
			output_hull.m_bounding_box.m_max = ToVec3(convex_hull.mBmax);
			
			ret.push_back(std::move(output_hull));
		}
	}
	else {
		assert(false, "The V-HACD ERROR for some reason!\n");
	}

	return ret;
}

}