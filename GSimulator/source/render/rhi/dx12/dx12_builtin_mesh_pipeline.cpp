#include "render/rhi/dx12/dx12_builtin_mesh_pipeline.h"

#include <d3dcompiler.h>

#include <array>
#include <cstring>
#include <cstdio>
#include <cstdint>

namespace GComponent {

namespace {

constexpr uint32_t kDx12BuiltinTextureBindingCount = 16;

constexpr const char* kPosition3VertexShaderSource = R"(
cbuffer Matrices : register(b0)
{
	float4x4 projection;
	float4x4 view;
};

struct VSInput
{
	float3 position : POSITION;
};

struct VSOutput
{
	float4 position : SV_Position;
	float3 color : COLOR0;
	float2 texcoord : TEXCOORD0;
	float3 direction : TEXCOORD1;
};

VSOutput main(VSInput input)
{
	VSOutput output;
	const float4 world_position = float4(input.position, 1.0f);
	output.position = mul(projection, mul(view, world_position));
	output.color = float3(1.0f, 1.0f, 1.0f);
	output.texcoord = float2(0.0f, 0.0f);
	output.direction = input.position;
	return output;
}
)";

constexpr const char* kPositionNormalTexcoordVertexShaderSource = R"(
cbuffer Matrices : register(b0)
{
	float4x4 projection;
	float4x4 view;
};

struct VSInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD0;
};

struct VSOutput
{
	float4 position : SV_Position;
	float3 color : COLOR0;
	float2 texcoord : TEXCOORD0;
	float3 direction : TEXCOORD1;
};

VSOutput main(VSInput input)
{
	VSOutput output;
	const float4 world_position = float4(input.position, 1.0f);
	output.position = mul(projection, mul(view, world_position));
	output.color = float3(1.0f, 1.0f, 1.0f);
	output.texcoord = input.texcoord;
	output.direction = input.position;
	return output;
}
)";

constexpr const char* kPositionNormalTexcoordColorVertexShaderSource = R"(
cbuffer Matrices : register(b0)
{
	float4x4 projection;
	float4x4 view;
};

struct VSInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD0;
	float3 color : COLOR0;
};

struct VSOutput
{
	float4 position : SV_Position;
	float3 color : COLOR0;
	float2 texcoord : TEXCOORD0;
	float3 direction : TEXCOORD1;
};

VSOutput main(VSInput input)
{
	VSOutput output;
	const float4 world_position = float4(input.position, 1.0f);
	output.position = mul(projection, mul(view, world_position));
	output.color = input.color;
	output.texcoord = input.texcoord;
	output.direction = input.position;
	return output;
}
)";

constexpr const char* kSkyboxVertexShaderSource = R"(
cbuffer Matrices : register(b0)
{
	float4x4 projection;
	float4x4 view;
};

struct VSInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD0;
};

struct VSOutput
{
	float4 position : SV_Position;
	float3 color : COLOR0;
	float2 texcoord : TEXCOORD0;
	float3 direction : TEXCOORD1;
};

VSOutput main(VSInput input)
{
	VSOutput output;
	const float4 clip_position = mul(projection, float4(mul((float3x3)view, input.position), 1.0f));
	output.position = clip_position.xyww;
	output.color = float3(1.0f, 1.0f, 1.0f);
	output.texcoord = input.texcoord;
	output.direction = input.position;
	return output;
}
)";

constexpr const char* kMeshColorPixelShaderSource = R"(
struct PSInput
{
	float4 position : SV_Position;
	float3 color : COLOR0;
	float2 texcoord : TEXCOORD0;
	float3 direction : TEXCOORD1;
};

float4 main(PSInput input) : SV_Target0
{
	return float4(input.color, 1.0f);
}
)";

constexpr const char* kMeshTexturedPixelShaderSource = R"(
Texture2D bound_texture : register(t0);
SamplerState linear_wrap_sampler : register(s0);

struct PSInput
{
	float4 position : SV_Position;
	float3 color : COLOR0;
	float2 texcoord : TEXCOORD0;
	float3 direction : TEXCOORD1;
};

float4 main(PSInput input) : SV_Target0
{
	return bound_texture.Sample(linear_wrap_sampler, input.texcoord) * float4(input.color, 1.0f);
}
)";

constexpr const char* kSkyboxPixelShaderSource = R"(
TextureCube cubemap_texture : register(t7);
SamplerState linear_clamp_sampler : register(s1);

struct PSInput
{
	float4 position : SV_Position;
	float3 color : COLOR0;
	float2 texcoord : TEXCOORD0;
	float3 direction : TEXCOORD1;
};

float4 main(PSInput input) : SV_Target0
{
	return cubemap_texture.Sample(linear_clamp_sampler, normalize(input.direction));
}
)";

bool CompileShader(const char* source,
				   const char* source_name,
				   const char* entry_point,
				   const char* target,
				   Microsoft::WRL::ComPtr<ID3DBlob>& bytecode)
{
	if (source == nullptr || source_name == nullptr || entry_point == nullptr || target == nullptr) {
		return false;
	}

	UINT compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
	compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	Microsoft::WRL::ComPtr<ID3DBlob> errors;
	const HRESULT hr = D3DCompile(
		source,
		std::strlen(source),
		source_name,
		nullptr,
		nullptr,
		entry_point,
		target,
		compile_flags,
		0,
		&bytecode,
		&errors);
	if (FAILED(hr)) {
		if (errors != nullptr) {
			std::printf("Dx12BuiltinMeshPipelineLibrary: failed to compile %s (%s): %s\n",
						source_name,
						target,
						static_cast<const char*>(errors->GetBufferPointer()));
		}
		else {
			std::printf("Dx12BuiltinMeshPipelineLibrary: failed to compile %s (%s), HRESULT=0x%08lx.\n",
						source_name,
						target,
						static_cast<unsigned long>(hr));
		}
		return false;
	}
	return true;
}

D3D12_BLEND_DESC CreateBlendDesc(bool enable_alpha_blend)
{
	D3D12_BLEND_DESC blend_desc{};
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;

	D3D12_RENDER_TARGET_BLEND_DESC& render_target = blend_desc.RenderTarget[0];
	render_target.BlendEnable = enable_alpha_blend ? TRUE : FALSE;
	render_target.LogicOpEnable = FALSE;
	render_target.SrcBlend = enable_alpha_blend ? D3D12_BLEND_SRC_ALPHA : D3D12_BLEND_ONE;
	render_target.DestBlend = enable_alpha_blend ? D3D12_BLEND_INV_SRC_ALPHA : D3D12_BLEND_ZERO;
	render_target.BlendOp = D3D12_BLEND_OP_ADD;
	render_target.SrcBlendAlpha = enable_alpha_blend ? D3D12_BLEND_ONE : D3D12_BLEND_ONE;
	render_target.DestBlendAlpha = enable_alpha_blend ? D3D12_BLEND_INV_SRC_ALPHA : D3D12_BLEND_ZERO;
	render_target.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	render_target.LogicOp = D3D12_LOGIC_OP_NOOP;
	render_target.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	return blend_desc;
}

D3D12_RASTERIZER_DESC CreateRasterizerDesc(D3D12_FILL_MODE fill_mode, D3D12_CULL_MODE cull_mode)
{
	D3D12_RASTERIZER_DESC rasterizer_desc{};
	rasterizer_desc.FillMode = fill_mode;
	rasterizer_desc.CullMode = cull_mode;
	rasterizer_desc.FrontCounterClockwise = FALSE;
	rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizer_desc.DepthClipEnable = TRUE;
	rasterizer_desc.MultisampleEnable = FALSE;
	rasterizer_desc.AntialiasedLineEnable = FALSE;
	rasterizer_desc.ForcedSampleCount = 0;
	rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	return rasterizer_desc;
}

D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc(bool enable_depth_test, D3D12_COMPARISON_FUNC depth_func)
{
	D3D12_DEPTH_STENCIL_DESC depth_stencil_desc{};
	depth_stencil_desc.DepthEnable = enable_depth_test ? TRUE : FALSE;
	depth_stencil_desc.DepthWriteMask = enable_depth_test ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = enable_depth_test ? depth_func : D3D12_COMPARISON_FUNC_ALWAYS;
	depth_stencil_desc.StencilEnable = FALSE;
	depth_stencil_desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depth_stencil_desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	depth_stencil_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	depth_stencil_desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	return depth_stencil_desc;
}

D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc(RhiVertexLayout vertex_layout)
{
	static constexpr std::array<D3D12_INPUT_ELEMENT_DESC, 1> kPosition3InputLayout = {
		D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	static constexpr std::array<D3D12_INPUT_ELEMENT_DESC, 3> kPositionNormalTexcoordInputLayout = {
		D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	static constexpr std::array<D3D12_INPUT_ELEMENT_DESC, 4> kPositionNormalTexcoordColorInputLayout = {
		D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	switch (vertex_layout) {
	case RhiVertexLayout::PositionNormalTexcoord:
		return D3D12_INPUT_LAYOUT_DESC{
			kPositionNormalTexcoordInputLayout.data(),
			static_cast<UINT>(kPositionNormalTexcoordInputLayout.size())
		};
	case RhiVertexLayout::PositionNormalTexcoordColor:
		return D3D12_INPUT_LAYOUT_DESC{
			kPositionNormalTexcoordColorInputLayout.data(),
			static_cast<UINT>(kPositionNormalTexcoordColorInputLayout.size())
		};
	case RhiVertexLayout::Position3:
	default:
		return D3D12_INPUT_LAYOUT_DESC{
			kPosition3InputLayout.data(),
			static_cast<UINT>(kPosition3InputLayout.size())
		};
	}
}

} // namespace

bool Dx12BuiltinMeshPipelineLibrary::Initialize(ID3D12Device* device)
{
	Reset();
	if (device == nullptr) {
		return false;
	}

	device_ = device;
	return CreateRootSignature() && CompileShaders();
}

void Dx12BuiltinMeshPipelineLibrary::Reset()
{
	pipeline_states_.clear();
	skybox_pixel_shader_.Reset();
	mesh_textured_pixel_shader_.Reset();
	mesh_color_pixel_shader_.Reset();
	skybox_vertex_shader_.Reset();
	position_normal_texcoord_color_vertex_shader_.Reset();
	position_normal_texcoord_vertex_shader_.Reset();
	position3_vertex_shader_.Reset();
	root_signature_.Reset();
	device_ = nullptr;
}

ID3D12RootSignature* Dx12BuiltinMeshPipelineLibrary::GetRootSignature() const
{
	return root_signature_.Get();
}

ID3D12PipelineState* Dx12BuiltinMeshPipelineLibrary::GetOrCreatePipeline(const Dx12BuiltinMeshPipelineDesc& desc, std::string* error_message)
{
	if (device_ == nullptr || root_signature_ == nullptr) {
		if (error_message != nullptr) {
			*error_message = "builtin mesh pipeline library is not ready";
		}
		return nullptr;
	}
	if (desc.shader != Dx12BuiltinPassShader::DepthOnly && desc.render_target_format == DXGI_FORMAT_UNKNOWN) {
		if (error_message != nullptr) {
			*error_message = "builtin mesh render-target format is unavailable";
		}
		return nullptr;
	}

	const uint64_t pipeline_key = MakePipelineKey(desc);
	const auto existing = pipeline_states_.find(pipeline_key);
	if (existing != pipeline_states_.end()) {
		return existing->second.Get();
	}

	ID3DBlob* vertex_shader = nullptr;
	ID3DBlob* pixel_shader = nullptr;
	if (desc.shader == Dx12BuiltinPassShader::Skybox) {
		vertex_shader = skybox_vertex_shader_.Get();
		pixel_shader = skybox_pixel_shader_.Get();
	}
	else {
		vertex_shader = position3_vertex_shader_.Get();
		switch (desc.vertex_layout) {
		case RhiVertexLayout::PositionNormalTexcoord:
			vertex_shader = position_normal_texcoord_vertex_shader_.Get();
			break;
		case RhiVertexLayout::PositionNormalTexcoordColor:
			vertex_shader = position_normal_texcoord_color_vertex_shader_.Get();
			break;
		case RhiVertexLayout::Position3:
		default:
			break;
		}

		switch (desc.shader) {
		case Dx12BuiltinPassShader::MeshTextured2D:
			pixel_shader = mesh_textured_pixel_shader_.Get();
			break;
		case Dx12BuiltinPassShader::MeshColor:
			pixel_shader = mesh_color_pixel_shader_.Get();
			break;
		case Dx12BuiltinPassShader::DepthOnly:
			pixel_shader = nullptr;
			break;
		case Dx12BuiltinPassShader::Skybox:
			break;
		}
	}

	if (vertex_shader == nullptr || (desc.shader != Dx12BuiltinPassShader::DepthOnly && pixel_shader == nullptr)) {
		if (!CompileShaders()) {
			if (error_message != nullptr) {
				*error_message = "builtin mesh shaders are unavailable";
			}
			return nullptr;
		}

		if (desc.shader == Dx12BuiltinPassShader::Skybox) {
			vertex_shader = skybox_vertex_shader_.Get();
			pixel_shader = skybox_pixel_shader_.Get();
		}
		else {
			vertex_shader = position3_vertex_shader_.Get();
			switch (desc.vertex_layout) {
			case RhiVertexLayout::PositionNormalTexcoord:
				vertex_shader = position_normal_texcoord_vertex_shader_.Get();
				break;
			case RhiVertexLayout::PositionNormalTexcoordColor:
				vertex_shader = position_normal_texcoord_color_vertex_shader_.Get();
				break;
			case RhiVertexLayout::Position3:
			default:
				break;
			}

			switch (desc.shader) {
			case Dx12BuiltinPassShader::MeshTextured2D:
				pixel_shader = mesh_textured_pixel_shader_.Get();
				break;
			case Dx12BuiltinPassShader::MeshColor:
				pixel_shader = mesh_color_pixel_shader_.Get();
				break;
			case Dx12BuiltinPassShader::DepthOnly:
				pixel_shader = nullptr;
				break;
			case Dx12BuiltinPassShader::Skybox:
				break;
			}
		}
	}

	if (vertex_shader == nullptr || (desc.shader != Dx12BuiltinPassShader::DepthOnly && pixel_shader == nullptr)) {
		if (error_message != nullptr) {
			*error_message = "builtin mesh shaders are unavailable";
		}
		return nullptr;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_desc{};
	pipeline_desc.pRootSignature = root_signature_.Get();
	pipeline_desc.VS.pShaderBytecode = vertex_shader->GetBufferPointer();
	pipeline_desc.VS.BytecodeLength = vertex_shader->GetBufferSize();
	pipeline_desc.BlendState = CreateBlendDesc(desc.enable_alpha_blend);
	pipeline_desc.SampleMask = UINT_MAX;
	pipeline_desc.RasterizerState = CreateRasterizerDesc(desc.fill_mode, desc.cull_mode);
	pipeline_desc.DepthStencilState = CreateDepthStencilDesc(desc.enable_depth_test, desc.depth_func);
	pipeline_desc.InputLayout = GetInputLayoutDesc(desc.vertex_layout);
	pipeline_desc.PrimitiveTopologyType = desc.topology_type;
	if (desc.shader == Dx12BuiltinPassShader::DepthOnly) {
		pipeline_desc.NumRenderTargets = 0;
		pipeline_desc.PS = {};
	}
	else {
		pipeline_desc.NumRenderTargets = 1;
		pipeline_desc.RTVFormats[0] = desc.render_target_format;
		pipeline_desc.PS.pShaderBytecode = pixel_shader->GetBufferPointer();
		pipeline_desc.PS.BytecodeLength = pixel_shader->GetBufferSize();
	}
	pipeline_desc.DSVFormat = desc.depth_stencil_format;
	pipeline_desc.SampleDesc.Count = 1;
	pipeline_desc.SampleDesc.Quality = 0;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state;
	const HRESULT hr = device_->CreateGraphicsPipelineState(&pipeline_desc, IID_PPV_ARGS(&pipeline_state));
	if (FAILED(hr)) {
		if (error_message != nullptr) {
			*error_message = "ID3D12Device::CreateGraphicsPipelineState failed";
		}
		std::printf("Dx12BuiltinMeshPipelineLibrary: CreateGraphicsPipelineState failed (HRESULT=0x%08lx).\n",
					static_cast<unsigned long>(hr));
		return nullptr;
	}

	const auto [iter, inserted] = pipeline_states_.emplace(pipeline_key, std::move(pipeline_state));
	(void)inserted;
	return iter->second.Get();
}

bool Dx12BuiltinMeshPipelineLibrary::CreateRootSignature()
{
	if (device_ == nullptr) {
		return false;
	}

	D3D12_DESCRIPTOR_RANGE texture_range{};
	texture_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	texture_range.NumDescriptors = kDx12BuiltinTextureBindingCount;
	texture_range.BaseShaderRegister = 0;
	texture_range.RegisterSpace = 0;
	texture_range.OffsetInDescriptorsFromTableStart = 0;

	std::array<D3D12_ROOT_PARAMETER, 2> root_parameters{};
	root_parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	root_parameters[0].Descriptor.ShaderRegister = 0;
	root_parameters[0].Descriptor.RegisterSpace = 0;
	root_parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	root_parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	root_parameters[1].DescriptorTable.NumDescriptorRanges = 1;
	root_parameters[1].DescriptorTable.pDescriptorRanges = &texture_range;
	root_parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	std::array<D3D12_STATIC_SAMPLER_DESC, 2> static_samplers{};
	static_samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	static_samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	static_samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	static_samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	static_samplers[0].MipLODBias = 0.0f;
	static_samplers[0].MaxAnisotropy = 1;
	static_samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	static_samplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	static_samplers[0].MinLOD = 0.0f;
	static_samplers[0].ShaderRegister = 0;
	static_samplers[0].RegisterSpace = 0;
	static_samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	static_samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	static_samplers[1] = static_samplers[0];
	static_samplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	static_samplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	static_samplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	static_samplers[1].ShaderRegister = 1;

	D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
	root_signature_desc.NumParameters = static_cast<UINT>(root_parameters.size());
	root_signature_desc.pParameters = root_parameters.data();
	root_signature_desc.NumStaticSamplers = static_cast<UINT>(static_samplers.size());
	root_signature_desc.pStaticSamplers = static_samplers.data();
	root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> serialized_root_signature;
	Microsoft::WRL::ComPtr<ID3DBlob> errors;
	const HRESULT serialize_hr = D3D12SerializeRootSignature(
		&root_signature_desc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&serialized_root_signature,
		&errors);
	if (FAILED(serialize_hr)) {
		if (errors != nullptr) {
			std::printf("Dx12BuiltinMeshPipelineLibrary: root signature serialization failed: %s\n",
						static_cast<const char*>(errors->GetBufferPointer()));
		}
		return false;
	}

	const HRESULT create_hr = device_->CreateRootSignature(
		0,
		serialized_root_signature->GetBufferPointer(),
		serialized_root_signature->GetBufferSize(),
		IID_PPV_ARGS(&root_signature_));
	if (FAILED(create_hr)) {
		std::printf("Dx12BuiltinMeshPipelineLibrary: CreateRootSignature failed (HRESULT=0x%08lx).\n",
					static_cast<unsigned long>(create_hr));
		return false;
	}
	return true;
}

bool Dx12BuiltinMeshPipelineLibrary::CompileShaders()
{
	return CompileShader(kPosition3VertexShaderSource, "dx12_builtin_mesh_position3.hlsl", "main", "vs_5_0", position3_vertex_shader_)
		&& CompileShader(kPositionNormalTexcoordVertexShaderSource, "dx12_builtin_mesh_position_normal_texcoord.hlsl", "main", "vs_5_0", position_normal_texcoord_vertex_shader_)
		&& CompileShader(kPositionNormalTexcoordColorVertexShaderSource, "dx12_builtin_mesh_position_normal_texcoord_color.hlsl", "main", "vs_5_0", position_normal_texcoord_color_vertex_shader_)
		&& CompileShader(kSkyboxVertexShaderSource, "dx12_builtin_skybox_vertex.hlsl", "main", "vs_5_0", skybox_vertex_shader_)
		&& CompileShader(kMeshColorPixelShaderSource, "dx12_builtin_mesh_color_pixel.hlsl", "main", "ps_5_0", mesh_color_pixel_shader_)
		&& CompileShader(kMeshTexturedPixelShaderSource, "dx12_builtin_mesh_textured_pixel.hlsl", "main", "ps_5_0", mesh_textured_pixel_shader_)
		&& CompileShader(kSkyboxPixelShaderSource, "dx12_builtin_skybox_pixel.hlsl", "main", "ps_5_0", skybox_pixel_shader_);
}

uint64_t Dx12BuiltinMeshPipelineLibrary::MakePipelineKey(const Dx12BuiltinMeshPipelineDesc& desc) const
{
	uint64_t key = 1469598103934665603ull;
	auto combine = [&key](uint64_t value) {
		key ^= value;
		key *= 1099511628211ull;
	};

	combine(static_cast<uint64_t>(desc.render_target_format));
	combine(static_cast<uint64_t>(desc.depth_stencil_format));
	combine(static_cast<uint64_t>(desc.vertex_layout));
	combine(static_cast<uint64_t>(desc.topology_type));
	combine(static_cast<uint64_t>(desc.shader));
	combine(static_cast<uint64_t>(desc.fill_mode));
	combine(static_cast<uint64_t>(desc.cull_mode));
	combine(static_cast<uint64_t>(desc.enable_alpha_blend ? 1u : 0u));
	combine(static_cast<uint64_t>(desc.enable_depth_test ? 1u : 0u));
	combine(static_cast<uint64_t>(desc.depth_func));
	return key;
}

} // namespace GComponent
