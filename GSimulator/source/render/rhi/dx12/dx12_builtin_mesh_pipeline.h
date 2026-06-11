#ifndef GSIM_RENDER_RHI_DX12_BUILTIN_MESH_PIPELINE_H
#define GSIM_RENDER_RHI_DX12_BUILTIN_MESH_PIPELINE_H

#include "render/rhi/dx12/dx12_common.h"
#include "render/rhi/rhi_resource.h"

#include <d3dcommon.h>

#include <string>
#include <unordered_map>

namespace GComponent {

enum class Dx12BuiltinPassShader {
	MeshColor,
	MeshTextured2D,
	Skybox,
	DepthOnly
};

struct Dx12BuiltinMeshPipelineDesc {
	DXGI_FORMAT render_target_format = DXGI_FORMAT_UNKNOWN;
	DXGI_FORMAT depth_stencil_format = DXGI_FORMAT_UNKNOWN;
	RhiVertexLayout vertex_layout = RhiVertexLayout::Position3;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Dx12BuiltinPassShader shader = Dx12BuiltinPassShader::MeshColor;
	D3D12_FILL_MODE fill_mode = D3D12_FILL_MODE_SOLID;
	D3D12_CULL_MODE cull_mode = D3D12_CULL_MODE_NONE;
	bool enable_alpha_blend = false;
	bool enable_depth_test = false;
	D3D12_COMPARISON_FUNC depth_func = D3D12_COMPARISON_FUNC_LESS;
};

class Dx12BuiltinMeshPipelineLibrary {
public:
	bool Initialize(ID3D12Device* device);
	void Reset();

	[[nodiscard]] ID3D12RootSignature* GetRootSignature() const;
	[[nodiscard]] ID3D12PipelineState* GetOrCreatePipeline(const Dx12BuiltinMeshPipelineDesc& desc, std::string* error_message = nullptr);

private:
	bool CreateRootSignature();
	bool CompileShaders();
	uint64_t MakePipelineKey(const Dx12BuiltinMeshPipelineDesc& desc) const;

private:
	ID3D12Device* device_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_;
	Microsoft::WRL::ComPtr<ID3DBlob> position3_vertex_shader_;
	Microsoft::WRL::ComPtr<ID3DBlob> position_normal_texcoord_vertex_shader_;
	Microsoft::WRL::ComPtr<ID3DBlob> position_normal_texcoord_color_vertex_shader_;
	Microsoft::WRL::ComPtr<ID3DBlob> skybox_vertex_shader_;
	Microsoft::WRL::ComPtr<ID3DBlob> mesh_color_pixel_shader_;
	Microsoft::WRL::ComPtr<ID3DBlob> mesh_textured_pixel_shader_;
	Microsoft::WRL::ComPtr<ID3DBlob> skybox_pixel_shader_;
	std::unordered_map<uint64_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> pipeline_states_;
};

} // namespace GComponent

#endif // GSIM_RENDER_RHI_DX12_BUILTIN_MESH_PIPELINE_H
