#include "simplexmesh/linebox.h"

#include "manager/resourcemanager.h"

static int color_idx = 0;
static std::vector<glm::vec3> color_lists = {
	{1.0, 0.0, 0.0},
	{1.0, 1.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 1.0, 1.0},
	{0.0, 0.0, 1.0},
	{0.0, 1.0, 1.0},
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 0.0}
};

namespace GComponent {

GLineBox::GLineBox(const vec3& min_pos, const vec3& max_pos)
{
	color_idx = 0;
	verts.resize(8);
	Update(min_pos, max_pos);
}

void GLineBox::Draw(MyShader*)
{
	if (!isInit) return;
	MyShader* shader = ResourceManager::getInstance().GetShaderByName("linecolor");
	if (!shader) return;
	shader->use();
	shader->setMat4("model", glm::mat4(1.0f));
	rhi_device_->DrawMesh(mesh_, RhiPrimitiveTopology::LineStrip, static_cast<uint32_t>(indices_.size()));
}

void GLineBox::Update(const vec3& min_pos, const vec3& max_pos)
{
	verts[0].position = vec3(min_pos.x, min_pos.y, min_pos.z);
	verts[1].position = vec3(min_pos.x, min_pos.y, max_pos.z);
	verts[2].position = vec3(min_pos.x, max_pos.y, min_pos.z);
	verts[3].position = vec3(min_pos.x, max_pos.y, max_pos.z);
	verts[4].position = vec3(max_pos.x, min_pos.y, min_pos.z);
	verts[5].position = vec3(max_pos.x, min_pos.y, max_pos.z);
	verts[6].position = vec3(max_pos.x, max_pos.y, min_pos.z);
	verts[7].position = vec3(max_pos.x, max_pos.y, max_pos.z);

	for (auto& vert : verts) {
		vert.Color = color_lists[color_idx];
	}
	color_idx = (++color_idx % color_lists.size());

	if (isInit) {
		rhi_device_->UpdateMeshVertexData(mesh_, 0, sizeof(ColorVertex) * verts.size(), verts.data());
	}
}

void GLineBox::GLBufferInitialize()
{
	if (isInit) return;

	indices_ = {
		0, 1, 3, 2, 0,
		4, 5, 1, 5,
		4, 6, 7, 5, 7,
		3, 2, 6
	};

	mesh_ = rhi_device_->CreateMesh(RhiMeshDesc{
		.vertex_data = verts.data(),
		.vertex_data_size = sizeof(ColorVertex) * verts.size(),
		.vertex_count = verts.size(),
		.vertex_stride = sizeof(ColorVertex),
		.index_data = indices_.data(),
		.index_data_size = sizeof(uint32_t) * indices_.size(),
		.index_count = indices_.size(),
		.vertex_layout = RhiVertexLayout::PositionNormalTexcoordColor
	});
}

} // namespace GComponent
