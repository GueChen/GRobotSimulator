/**
 *  @file  Axis.h
 *  @brief Axis Resource File used for Graphic Program implementations etc.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date Apri 16, 2022
 **/
#include "Axis.h"

#include "GComponent/GNumerical.hpp"

using std::vector;
using namespace GComponent;

/*__________________ Abstract Axis ____________________*/
/// <summary>
/// Initialization the Axis Data including Vertex and Index
/// </summary>
/// <param name="segments">{int}   the Resolution about circle</param>
/// <param name="radius">  {float} the Radius of Cone </param>
void AbstractAxis::Init(int segments, float radius) {
	vertices_ = vector<Vertex>(0);
	indicies_ = vector<Triangle>(0);

	SetupVertexData(segments, radius);
	SetupIndexData(segments);

	std::tie(model_id_, VBO) = GMeshObject::genVABO(&vertices_[0], sizeof(Vertex) * vertices_.size());
	GMeshObject::EnableVertexAttrbArrays(3, 3, 2);
	EBO = GMeshObject::genEBO(indicies_);
}

/// <summary>
/// Draw acoording the shader
/// </summary>
/// <param name="shader"> {MyShader} normally including pick and axis </param>
/// <returns></returns>
void AbstractAxis::Draw(MyShader& shader) const noexcept {
	shader.use();
	glBindVertexArray(model_id_);
	glDrawElements(GL_TRIANGLES, 3 * indicies_.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void AbstractAxis::SetupXaxisCircle(int segments, float radius, float fixed_x) {
	for (int i = 0; i < segments; ++i) {
		Vertex		temp;
		glm::vec3&	pos		= temp.pos,
				 &	norm	= temp.norm;
		glm::vec2&	coord	= temp.coord;

		pos.x = fixed_x;
		pos.y = sin(i * 2 * MyPI / segments) * radius;
		pos.z = cos(i * 2 * MyPI / segments) * radius;

		norm.x  = norm.y  = norm.z = 
		coord.x = coord.y = 0.0f;
		vertices_.emplace_back(temp);
	}
}

void AbstractAxis::SetupYaxisVertex(const int count) {
	for (int i = 0; i < count; ++i) {
		Vertex			temp,
					&	ref			= vertices_[i];
		glm::vec3	&	pos			= temp.pos,
					&	norm		= temp.norm,
					&	ref_pos		= ref.pos,
					&	ref_norm	= ref.norm;
		glm::vec2	&	coord		= temp.coord;

		pos.z = -ref_pos.x; 
		pos.x = ref_pos.y;
		pos.y = ref_pos.z;

		norm.z = -ref_norm.x;
		norm.x = ref_norm.y;
		norm.y = ref_norm.z;

		coord.x = 1.0f;
		coord.y = 0.0f;
		vertices_.emplace_back(temp);
	}
}

void AbstractAxis::SetupZaxisVertex(const int count) {
	for (int i = 0; i < count; ++i) {
		Vertex			temp,
					&	ref			= vertices_[i];
		glm::vec3	&	pos			= temp.pos,
					&	norm		= temp.norm,
					&	ref_pos		= ref.pos,
					&	ref_norm	= ref.norm;
		glm::vec2	&	coord		= temp.coord;

		pos.y = -ref_pos.x;
		pos.z = ref_pos.y;
		pos.x = ref_pos.z;

		norm.y = -ref_norm.x;
		norm.z = ref_norm.y;
		norm.x = ref_norm.z;

		coord.x = 2.0f;
		coord.y = 0.0f;
		vertices_.emplace_back(temp);
	}
}

void AbstractAxis::LinkTriangle(int strided, int base, int i1, int i2, int i3) {
	unsigned first = strided + base + i1,
			 second = strided + base + i2,
			 third = strided + base + i3;
	indicies_.emplace_back(first, second, third);
}

void AbstractAxis::LinkSquare(int strided, int base, int i1, int i2, int i3, int i4) {
	LinkTriangle(strided, base, i1, i2, i3);
	LinkTriangle(strided, base, i4, i3, i2);
}

/*__________________ Translation Axis ____________________*/
TranslationAxis::TranslationAxis()
{}

void TranslationAxis::SetupVertexData(int segments, float radius)
{
	/* Make X axis shape */
	/* the Cylinder inner Circle */
	SetupXaxisCircle(segments, radius, 0.0f);
	/* the Cylinder outer Circle */
	SetupXaxisCircle(segments, radius, -1.5);
	SetupXaxisCircle(segments, radius * 4.0f, -1.5);
	/* the Cone Conical Point */
	{
		Vertex		temp;
		glm::vec3&	pos		= temp.pos,
				 &	norm	= temp.norm;
		glm::vec2&	coord	= temp.coord;

		pos.x = -1.65f;
		pos.y = pos.z = 0.0f;

		norm.x = -1.0f;
		norm.y = norm.z = 0.0f;
		coord.x = coord.y = 0.0f;
		vertices_.emplace_back(temp);
	}
	/* the Y-Z axis */
	int vertex_count = vertices_.size();
	/* the Y case */
	SetupYaxisVertex(vertex_count);
	SetupZaxisVertex(vertex_count);

}

void TranslationAxis::SetupIndexData(int segments)
{
	const int strided = segments * 3 + 1;
	/* caculate the index */
	for (int k = 0; k < 3; ++k) {
		unsigned vertex_stride = k * strided;
		/* The Cilindral part */
		for (int j = 0; j < 2; ++j)
		{
			for (int i = 0; i < segments - 1; ++i) {
				LinkSquare(vertex_stride, j * segments, i, i + 1, i + segments, i + segments + 1);
			}
			{
				LinkSquare(vertex_stride, j * segments, segments - 1, 0, 2 * segments - 1, segments);
			}
		}
		/* the cone Part */
		{
			for (int i = 0; i < segments - 1; ++i) {
				LinkTriangle(vertex_stride, 2 * segments, i, i + 1, segments);
			}
			{
				LinkTriangle(vertex_stride, 2 * segments, segments - 1, 0, segments);
			}
		}
	}
}

/*__________________ Rotation Axis ____________________*/
RotationAxis::RotationAxis()
{}

void RotationAxis::SetupVertexData(int segments, float radius)
{
	/* Make X Circle */
	SetupXaxisCircle(segments, radius, 0.0f);
	SetupXaxisCircle(segments, 1.07f * radius, 0.0f);
	
	const int vertex_count = vertices_.size();
	/* Make Y-Z Vertex Setup */
	SetupYaxisVertex(vertex_count);
	SetupZaxisVertex(vertex_count);
}

void RotationAxis::SetupIndexData(int segments)
{
	const int strided = 2 * segments;
	for (int k = 0; k < 3; ++k) {
		int vertex_stride = k * strided;
		for (int i = 0; i < segments - 1; ++i) {
			LinkSquare(vertex_stride, 0, i, i + 1, i + segments, i + segments + 1);	
		}
		LinkSquare(vertex_stride, 0, segments - 1, 0, 2 * segments - 1, segments);
	}
}

/*__________________ Scale Axis ____________________*/
ScaleAxis::ScaleAxis()
{}

void ScaleAxis::SetupVertexData(int segments, float radius)
{
	/* Make X axis shape */
	/* the Cylinder inner Circle */
	SetupXaxisCircle(segments, radius, 0.0f);
	/* the Cylinder outer Circle */
	SetupXaxisCircle(segments, radius, -1.5f);

	/* the Block part Vertex */
	glm::vec3 base_pos(-1.5f, 0.0f, 0.0f);
	float half = radius * 5.0f;
	auto emplace_vertex_in_x = [&vertices = vertices_, &base_pos = base_pos](float x, float y, float z) {
		Vertex		temp;
		glm::vec3&	pos		= temp.pos,
				 &	norm	= temp.norm;
		glm::vec2&	coord	= temp.coord;

		pos = base_pos + glm::vec3(x, y, z);
		norm = glm::zero<glm::vec3>();
		coord.x = coord.y = 0.0f;
		vertices.emplace_back(temp);
	};
	emplace_vertex_in_x( half,  half,  half);
	emplace_vertex_in_x( half, -half,  half);
	emplace_vertex_in_x( half,  half, -half);
	emplace_vertex_in_x( half, -half, -half);
	emplace_vertex_in_x(-half,  half,  half);
	emplace_vertex_in_x(-half, -half,  half);
	emplace_vertex_in_x(-half,  half, -half);
	emplace_vertex_in_x(-half, -half, -half);
	
	/* set Y-Z Vertex */
	const int vertex_count = vertices_.size();
	/* Y Vertex setup */
	SetupYaxisVertex(vertex_count);
	/* Z Vertex setup */
	SetupZaxisVertex(vertex_count);
}

void ScaleAxis::SetupIndexData(int segments)
{
	const int stride = 2 * segments + 8;
	for (int k = 0; k < 3; ++k) {
		/* Link the Cylindral Part */
		int vertex_stride = k * stride;
		{
			for (int i = 0; i < segments - 1; ++i) {
				LinkSquare(vertex_stride, 0, i, i + 1, i + segments, i + 1 + segments);
			}
			LinkSquare(vertex_stride, 0, segments - 1, 0, 2 * segments - 1, segments);
		}
		/* Link the Block Part */
		LinkSquare(vertex_stride, 2 * segments, 0, 1, 2, 3);
		LinkSquare(vertex_stride, 2 * segments, 4, 5, 6, 7);
		LinkSquare(vertex_stride, 2 * segments, 0, 1, 4, 5);
		LinkSquare(vertex_stride, 2 * segments, 2, 3, 6, 7);
		LinkSquare(vertex_stride, 2 * segments, 1, 3, 5, 7);
		LinkSquare(vertex_stride, 2 * segments, 0, 2, 4, 6);
	}
}