/**
 *  @file	qtaxis.cpp
 *  @brief	Axis Resource File used for Graphic Program implementations etc.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date	Apri 26, 2022
 **/
#include "model/axis/qtaxis.h"

#include "manager/rendermanager.h"
#include "manager/resourcemanager.h"

size_t GComponent::QtGLTranslationAxis::count = 0;
size_t GComponent::QtGLRotationAxis::count	  = 0;
size_t GComponent::QtGLScaleAxis::count		  = 0;

/*__________________ Abstract Axis ____________________*/
/// <summary>
/// Initialization the Axis Data including Vertex and Index
/// </summary>
/// <param name="segments">{int}   the Resolution about circle</param>
/// <param name="radius">  {float} the Radius of Cone </param>
void GComponent::QtGLAbstractAxis::Init(int segments, float radius)
{
	ResourceManager::getInstance().RegisteredMesh(mesh_, new RenderMesh(SetupVertexData(segments, radius), SetupIndexData(segments), {}));
}

unsigned GComponent::QtGLAbstractAxis::GetStridedSize()
{
	return ResourceManager::getInstance().GetMeshByName(mesh_)->getElementSize() / 3;
}

void GComponent::QtGLAbstractAxis::tick(float delta_time)
{
	RenderManager::getInstance().EmplaceAuxiRenderCommand(name_, shader_, mesh_);
}

void GComponent::QtGLAbstractAxis::Draw()
{
	ResourceManager::getInstance().GetMeshByName(mesh_)->Draw();
}

void GComponent::QtGLAbstractAxis::setShaderProperty(MyShader& shader)
{
	shader.setMat4("model",	getModelMatrix());
	shader.setInt( "selected", static_cast<int>(selected_which_));
}

void GComponent::QtGLAbstractAxis::SetupXaxisCircle(int segments, float radius, float fixed_x, vector<Vertex> & vertices)
{
	for (int i = 0; i < segments; ++i) {
		Vertex		temp;
		glm::vec3&	pos		= temp.Position,
				 &	norm	= temp.Normal;
		glm::vec2&	coord	= temp.TexCoords;

		pos.x = fixed_x;
		pos.y = sin(i * 2 * MyPI / segments) * radius;
		pos.z = cos(i * 2 * MyPI / segments) * radius;

		norm.x  = norm.y  = norm.z = 
		coord.x = coord.y = 0.0f;
		vertices.emplace_back(temp);
	}
}

void GComponent::QtGLAbstractAxis::SetupYaxisVertex(const int count, vector<Vertex>& vertices)
{
	for (int i = 0; i < count; ++i) {
		Vertex			temp,
					&	ref			= vertices[i];
		glm::vec3	&	pos			= temp.Position,
					&	norm		= temp.Normal,
					&	ref_pos		= ref.Position,
					&	ref_norm	= ref.Normal;
		glm::vec2	&	coord		= temp.TexCoords;

		pos.z = ref_pos.y; 
		pos.x = ref_pos.z;
		pos.y = ref_pos.x;

		norm.z = ref_norm.y;
		norm.x = ref_norm.z;
		norm.y = ref_norm.x;

		coord.x = 1.0f;
		coord.y = 0.0f;
		vertices.emplace_back(temp);
	}
}

void GComponent::QtGLAbstractAxis::SetupZaxisVertex(const int count, vector<Vertex>& vertices)
{
	for (int i = 0; i < count; ++i) {
		Vertex			temp,
					&	ref			= vertices[i];
		glm::vec3	&	pos			= temp.Position,
					&	norm		= temp.Normal,
					&	ref_pos		= ref.Position,
					&	ref_norm	= ref.Normal;
		glm::vec2	&	coord		= temp.TexCoords;

		pos.z = ref_pos.x;
		pos.y = ref_pos.z;
		pos.x = ref_pos.y;

		norm.y = ref_norm.z;
		norm.z = ref_norm.x;
		norm.x = ref_norm.y;

		coord.x = 2.0f;
		coord.y = 0.0f;
		vertices.emplace_back(temp);
	}
}

void GComponent::QtGLAbstractAxis::LinkTriangle(int strided, int base, int i1, int i2, int i3, vector<Triangle>& indicies)
{
	unsigned first = strided + base + i1,
			 second = strided + base + i2,
			 third = strided + base + i3;
	indicies.emplace_back(first, second, third);
}

void GComponent::QtGLAbstractAxis::LinkSquare(int strided, int base, int i1, int i2, int i3, int i4, vector<Triangle>& indices)
{
	LinkTriangle(strided, base, i1, i2, i3, indices);
	LinkTriangle(strided, base, i4, i3, i2, indices);
}

/*__________________ Translation Axis ____________________*/
GComponent::QtGLTranslationAxis::QtGLTranslationAxis()
{
	name_ = "trans_axis_" + std::to_string(count);
	mesh_ = "trans_axis";
	++count;
}

std::vector<GComponent::Vertex> GComponent::QtGLTranslationAxis::SetupVertexData(int segments, float radius)
{
	std::vector<Vertex> vertices;
	/* Make X axis shape */
	/* the Cylinder inner Circle */
	SetupXaxisCircle(segments, radius, 0.0f, vertices);
	/* the Cylinder outer Circle */
	SetupXaxisCircle(segments, radius, 1.5, vertices);
	SetupXaxisCircle(segments, radius * 4.0f, 1.5, vertices);
	/* the Cone Conical Point */
	{
		Vertex		temp;
		glm::vec3&	pos		= temp.Position,
				 &	norm	= temp.Normal;
		glm::vec2&	coord	= temp.TexCoords;

		pos.x = 1.5f + 10.0f * radius;
		pos.y = pos.z = 0.0f;

		norm.x = 1.0f;
		norm.y = norm.z = 0.0f;
		coord.x = coord.y = 0.0f;
		vertices.emplace_back(temp);
	}
	/* the Y-Z axis */
	int vertex_count = vertices.size();
	/* the Y case */
	SetupYaxisVertex(vertex_count, vertices);
	SetupZaxisVertex(vertex_count, vertices);
	return vertices;
}

std::vector<GComponent::Triangle> GComponent::QtGLTranslationAxis::SetupIndexData(int segments)
{
	vector<Triangle> indices;
	const int strided = segments * 3 + 1;
	/* caculate the index */
	for (int k = 0; k < 3; ++k) {
		unsigned vertex_stride = k * strided;
		/* The Cilindral part */
		for (int j = 0; j < 2; ++j)
		{
			for (int i = 0; i < segments - 1; ++i) {
				LinkSquare(vertex_stride, j * segments, i, i + 1, i + segments, i + segments + 1, indices);
			}
			{
				LinkSquare(vertex_stride, j * segments, segments - 1, 0, 2 * segments - 1, segments, indices);
			}
		}
		/* the cone Part */
		{
			for (int i = 0; i < segments - 1; ++i) {
				LinkTriangle(vertex_stride, 2 * segments, i, i + 1, segments, indices);
			}
			{
				LinkTriangle(vertex_stride, 2 * segments, segments - 1, 0, segments, indices);
			}
		}
	}
	return indices;
}

/*__________________ Rotation Axis ____________________*/
GComponent::QtGLRotationAxis::QtGLRotationAxis()
{
	name_ = "rotate_axis_" + std::to_string(count);
	mesh_ = "rotate_axis"; 
	++count;
}

std::vector<GComponent::Vertex> GComponent::QtGLRotationAxis::SetupVertexData(int segments, float radius)
{
	vector<Vertex> vertices;
	/* Make X Circle */
	SetupXaxisCircle(segments, radius, 0.0f, vertices);
	SetupXaxisCircle(segments, 1.07f * radius, 0.0f, vertices);

	const int vertex_count = vertices.size();
	/* Make Y-Z Vertex Setup */
	SetupYaxisVertex(vertex_count, vertices);
	SetupZaxisVertex(vertex_count, vertices);
	return vertices;
}

std::vector<GComponent::Triangle> GComponent::QtGLRotationAxis::SetupIndexData(int segments)
{
	vector<Triangle> indices;
	const int strided = 2 * segments;
	for (int k = 0; k < 3; ++k) {
		int vertex_stride = k * strided;
		for (int i = 0; i < segments - 1; ++i) {
			LinkSquare(vertex_stride, 0, i, i + 1, i + segments, i + segments + 1, indices);
		}
		LinkSquare(vertex_stride, 0, segments - 1, 0, 2 * segments - 1, segments, indices);
	}
	return indices;
}

/*__________________ Scale Axis ____________________*/
GComponent::QtGLScaleAxis::QtGLScaleAxis()
{
	name_ = "scale_axis_" + std::to_string(count);
	mesh_ = "scale_axis"; 
	++count;
}

std::vector<GComponent::Vertex> GComponent::QtGLScaleAxis::SetupVertexData(int segments, float radius)
{
	vector<Vertex> vertices;
	/* Make X axis shape */
	/* the Cylinder inner Circle */
	SetupXaxisCircle(segments, radius, 0.0f, vertices);
	/* the Cylinder outer Circle */
	SetupXaxisCircle(segments, radius, 1.5f, vertices);

	/* the Block part Vertex */
	glm::vec3 base_pos(1.5f, 0.0f, 0.0f);
	float half = radius * 5.0f;
	auto emplace_vertex_in_x = [&vertices = vertices, &base_pos = base_pos](float x, float y, float z) {
		Vertex		temp;
		glm::vec3&	pos		= temp.Position,
				 &	norm	= temp.Normal;
		glm::vec2&	coord	= temp.TexCoords;

		pos = base_pos + glm::vec3(x, y, z);
		norm.x  = norm.y  = norm.z = 
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
	const int vertex_count = vertices.size();
	/* Y Vertex setup */
	SetupYaxisVertex(vertex_count, vertices);
	/* Z Vertex setup */
	SetupZaxisVertex(vertex_count, vertices);
	return vertices;
}

std::vector<GComponent::Triangle> GComponent::QtGLScaleAxis::SetupIndexData(int segments)
{
	vector<Triangle> indices;
	const int stride = 2 * segments + 8;
	for (int k = 0; k < 3; ++k) {
		/* Link the Cylindral Part */
		int vertex_stride = k * stride;
		{
			for (int i = 0; i < segments - 1; ++i) {
				LinkSquare(vertex_stride, 0, i, i + 1, i + segments, i + 1 + segments, indices);
			}
			LinkSquare(vertex_stride, 0, segments - 1, 0, 2 * segments - 1, segments, indices);
		}
		/* Link the Block Part */
		LinkSquare(vertex_stride, 2 * segments, 0, 1, 2, 3, indices);
		LinkSquare(vertex_stride, 2 * segments, 4, 5, 6, 7, indices);
		LinkSquare(vertex_stride, 2 * segments, 0, 1, 4, 5, indices);
		LinkSquare(vertex_stride, 2 * segments, 2, 3, 6, 7, indices);
		LinkSquare(vertex_stride, 2 * segments, 1, 3, 5, 7, indices);
		LinkSquare(vertex_stride, 2 * segments, 0, 2, 4, 6, indices);
	}
	return indices;
}
