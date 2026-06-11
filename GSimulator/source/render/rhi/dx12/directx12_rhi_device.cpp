#include "render/rhi/dx12/directx12_rhi_device.h"
#include "render/rhi/dx12/dx12_builtin_mesh_pipeline.h"
#include "render/rhi/rhi_factory.h"

#include <stb_image.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <utility>

namespace GComponent {

namespace {

constexpr uint32_t kInvalidDefaultFramebuffer = 0;
constexpr uint32_t kDefaultCbvHeapCapacity = 64;
constexpr uint32_t kDefaultOffscreenRtvHeapCapacity = 64;
constexpr uint32_t kDefaultOffscreenDsvHeapCapacity = 32;
constexpr uint32_t kInvalidDescriptorIndex = UINT32_MAX;
constexpr uint32_t kSwapChainBufferCount = 2;
constexpr DXGI_FORMAT kDefaultSwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
constexpr uint32_t kDx12BuiltinTextureBindingCount = 16;
constexpr uint32_t kDx12BuiltinTexture2DBinding = 0;
constexpr uint32_t kDx12BuiltinCubemapBinding = 7;

bool VertexLayoutSupportsTexcoords(RhiVertexLayout vertex_layout)
{
	return vertex_layout == RhiVertexLayout::PositionNormalTexcoord
		|| vertex_layout == RhiVertexLayout::PositionNormalTexcoordColor;
}

bool IsHardwareAdapter(IDXGIAdapter1* adapter)
{
	DXGI_ADAPTER_DESC1 description{};
	if (adapter == nullptr) {
		return false;
	}
	if (FAILED(adapter->GetDesc1(&description))) {
		return false;
	}
	return (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0;
}

RhiTextureDimension GetFramebufferTextureDimension(const RhiFramebufferCreateDesc& desc)
{
	if (desc.layers > 0) {
		return RhiTextureDimension::Texture2DArray;
	}
	return desc.attachment == RhiFramebufferAttachment::Color ? RhiTextureDimension::Texture2D : RhiTextureDimension::TextureCube;
}

RhiTextureFormat GetFramebufferAttachmentFormat(const RhiFramebufferCreateDesc& desc)
{
	switch (desc.attachment) {
	case RhiFramebufferAttachment::Depth:
		return RhiTextureFormat::Depth32Float;
	case RhiFramebufferAttachment::Cube:
	case RhiFramebufferAttachment::CubeMipmap:
		return RhiTextureFormat::Rgb16Float;
	case RhiFramebufferAttachment::Color:
	default:
		return RhiTextureFormat::Rgb32Float;
	}
}

DXGI_FORMAT ToDxgiResourceFormat(RhiTextureFormat format)
{
	switch (format) {
	case RhiTextureFormat::Rgb32Float:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case RhiTextureFormat::Rgb16Float:
	case RhiTextureFormat::Rgba16Float:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case RhiTextureFormat::Rg16Float:
		return DXGI_FORMAT_R16G16_FLOAT;
	case RhiTextureFormat::Rgba8Unorm:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case RhiTextureFormat::Depth32Float:
		return DXGI_FORMAT_R32_TYPELESS;
	case RhiTextureFormat::Depth24Stencil8:
		return DXGI_FORMAT_R24G8_TYPELESS;
	}
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT ToDxgiShaderResourceFormat(RhiTextureFormat format)
{
	switch (format) {
	case RhiTextureFormat::Depth32Float:
		return DXGI_FORMAT_R32_FLOAT;
	case RhiTextureFormat::Depth24Stencil8:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	default:
		return ToDxgiResourceFormat(format);
	}
}

DXGI_FORMAT ToDxgiRenderTargetFormat(RhiTextureFormat format)
{
	switch (format) {
	case RhiTextureFormat::Rgb32Float:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case RhiTextureFormat::Rgb16Float:
	case RhiTextureFormat::Rgba16Float:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case RhiTextureFormat::Rg16Float:
		return DXGI_FORMAT_R16G16_FLOAT;
	case RhiTextureFormat::Rgba8Unorm:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case RhiTextureFormat::Depth32Float:
	case RhiTextureFormat::Depth24Stencil8:
		return DXGI_FORMAT_UNKNOWN;
	}
	return DXGI_FORMAT_UNKNOWN;
}

DXGI_FORMAT ToDxgiDepthStencilFormat(RhiTextureFormat format)
{
	switch (format) {
	case RhiTextureFormat::Depth32Float:
		return DXGI_FORMAT_D32_FLOAT;
	case RhiTextureFormat::Depth24Stencil8:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

uint32_t GetTexturePixelStride(DXGI_FORMAT format)
{
	switch (format) {
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return sizeof(float) * 4;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return sizeof(uint16_t) * 4;
	case DXGI_FORMAT_R16G16_FLOAT:
		return sizeof(uint16_t) * 2;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return sizeof(uint8_t) * 4;
	default:
		return 0;
	}
}

float HalfToFloat(uint16_t value)
{
	const uint32_t sign = (value & 0x8000u) << 16;
	uint32_t exponent = (value >> 10) & 0x1Fu;
	uint32_t mantissa = value & 0x03FFu;

	if (exponent == 0) {
		if (mantissa == 0) {
			return std::bit_cast<float>(sign);
		}
		int32_t adjusted_exponent = -14;
		while ((mantissa & 0x0400u) == 0) {
			mantissa <<= 1;
			--adjusted_exponent;
		}
		mantissa &= 0x03FFu;
		const uint32_t bits = sign | (static_cast<uint32_t>(adjusted_exponent + 127) << 23) | (mantissa << 13);
		return std::bit_cast<float>(bits);
	}
	else if (exponent == 31) {
		return std::bit_cast<float>(sign | 0x7F800000u | (mantissa << 13));
	}

	exponent = exponent + (127 - 15);
	const uint32_t bits = sign | (exponent << 23) | (mantissa << 13);
	return std::bit_cast<float>(bits);
}

uint32_t GetTextureSubresourceCount(const RhiTextureDesc& desc)
{
	const uint32_t mip_levels = static_cast<uint32_t>(std::max(desc.mip_levels, 1));
	switch (desc.dimension) {
	case RhiTextureDimension::Texture2D:
		return mip_levels;
	case RhiTextureDimension::Texture2DArray:
		return mip_levels * static_cast<uint32_t>(std::max(desc.layers, 1));
	case RhiTextureDimension::TextureCube:
		return mip_levels * 6u;
	}
	return 0;
}

std::vector<DirectX12RhiDevice::TextureUploadSubresource> BuildTextureUploadSubresources(const RhiTextureDesc& desc,
	size_t pixel_stride,
	const void* initial_data)
{
	std::vector<DirectX12RhiDevice::TextureUploadSubresource> subresources;
	if (initial_data == nullptr || pixel_stride == 0 || desc.width <= 0 || desc.height <= 0) {
		return subresources;
	}

	const uint32_t mip_levels = static_cast<uint32_t>(std::max(desc.mip_levels, 1));
	const uint32_t array_size = desc.dimension == RhiTextureDimension::TextureCube
		? 6u
		: static_cast<uint32_t>(std::max(desc.layers, 1));
	subresources.reserve(array_size * mip_levels);

	const auto* bytes = static_cast<const std::byte*>(initial_data);
	size_t byte_offset = 0;
	for (uint32_t array_index = 0; array_index < array_size; ++array_index) {
		for (uint32_t mip_index = 0; mip_index < mip_levels; ++mip_index) {
			const size_t width = static_cast<size_t>(std::max(desc.width >> mip_index, 1));
			const size_t height = static_cast<size_t>(std::max(desc.height >> mip_index, 1));
			const size_t row_pitch = width * pixel_stride;
			const size_t slice_pitch = row_pitch * height;
			subresources.push_back(DirectX12RhiDevice::TextureUploadSubresource{
				.data = bytes + byte_offset,
				.row_pitch = row_pitch,
				.slice_pitch = slice_pitch
			});
			byte_offset += slice_pitch;
		}
	}

	return subresources;
}

} // namespace

std::optional<RhiBufferHandle> DirectX12RhiDevice::GetUniformBufferBinding(uint32_t binding) const
{
	const auto iter = uniform_buffer_bindings_.find(binding);
	if (iter == uniform_buffer_bindings_.end()) {
		return std::nullopt;
	}
	return iter->second;
}

DirectX12RhiDevice::DirectX12RhiDevice():
	DirectX12RhiDevice(RhiDeviceInitConfig{})
{
}

DirectX12RhiDevice::DirectX12RhiDevice(const RhiDeviceInitConfig& init_config):
	init_config_(init_config)
{
	init_config_.backend = RhiBackendType::DirectX12;
	capabilities_.backend = RhiBackendType::DirectX12;
}

DirectX12RhiDevice::~DirectX12RhiDevice()
{
	ResetDeviceState();
}

RhiBackendType DirectX12RhiDevice::GetBackendType() const
{
	return RhiBackendType::DirectX12;
}

void DirectX12RhiDevice::Initialize()
{
	Initialize(init_config_);
}

void DirectX12RhiDevice::Initialize(const RhiDeviceInitConfig& config)
{
	ResetDeviceState();

	init_config_ = config;
	init_config_.backend = RhiBackendType::DirectX12;
	capabilities_ = {};
	capabilities_.backend = RhiBackendType::DirectX12;

	EnableDebugLayerIfRequested();

	if (!CreateFactory() || !CreateAdapter() || !CreateDevice() || !CreateCommandQueue() || !CreateCommandObjects() || !CreateDescriptorHeaps() || !CreateFence() || !CreatePresentSurfaceResources() || !CreateBuiltinMeshPipelineResources()) {
		ResetDeviceState();
		return;
	}

	RefreshCapabilities();
	initialized_ = true;
}

void DirectX12RhiDevice::InitializeForSurface(const RhiDeviceInitConfig& config, const RhiPresentSurfaceDesc& surface)
{
	RhiDeviceInitConfig surface_config = config;
	surface_config.present_surface = surface;
	Initialize(surface_config);
}

const RhiDeviceInitConfig& DirectX12RhiDevice::GetInitConfig() const
{
	return init_config_;
}

RhiDeviceCapabilities DirectX12RhiDevice::GetCapabilities() const
{
	return capabilities_;
}

void DirectX12RhiDevice::Enable(RhiCapability capability)
{
	switch (capability) {
	case RhiCapability::DepthTest:
		depth_test_enabled_ = true;
		break;
	case RhiCapability::Blend:
		blend_enabled_ = true;
		break;
	case RhiCapability::Multisample:
		multisample_enabled_ = true;
		break;
	case RhiCapability::CullFace:
		cull_face_enabled_ = true;
		break;
	}
}

void DirectX12RhiDevice::Disable(RhiCapability capability)
{
	switch (capability) {
	case RhiCapability::DepthTest:
		depth_test_enabled_ = false;
		break;
	case RhiCapability::Blend:
		blend_enabled_ = false;
		break;
	case RhiCapability::Multisample:
		multisample_enabled_ = false;
		break;
	case RhiCapability::CullFace:
		cull_face_enabled_ = false;
		break;
	}
}

void DirectX12RhiDevice::SetDepthFunc(RhiDepthFunc func)
{
	depth_func_ = func;
}

void DirectX12RhiDevice::SetCullFace(RhiCullFace face)
{
	cull_face_ = face;
}

void DirectX12RhiDevice::SetPolygonMode(RhiPolygonMode mode)
{
	polygon_mode_ = mode;
}

void DirectX12RhiDevice::SetBlendAlpha()
{
	blend_alpha_enabled_ = true;
}

void DirectX12RhiDevice::SetLineWidth(float width)
{
	line_width_ = width;
}

void DirectX12RhiDevice::SetViewport(const RhiViewport& viewport)
{
	viewport_ = viewport;
}

void DirectX12RhiDevice::SetClearColor(const RhiClearColor& color)
{
	clear_color_ = color;
}

void DirectX12RhiDevice::Clear(RhiClearFlags flags)
{
	if (flags == RhiClearFlags::None) {
		return;
	}

	const auto& draw_binding = GetFramebufferBindingState(RhiFramebufferBindTarget::Draw);

	if (!ResetCommandListForRecording()) {
		return;
	}

	FramebufferPassState pass_state{};
	if (!BeginFramebufferPass(draw_binding, pass_state)) {
		return;
	}

	const float clear_color[] = { clear_color_.r, clear_color_.g, clear_color_.b, clear_color_.a };
	if (HasFlag(flags, RhiClearFlags::Color)) {
		for (uint32_t i = 0; i < pass_state.rtv_count; ++i) {
			command_list_->ClearRenderTargetView(pass_state.rtv_handles[i], clear_color, 0, nullptr);
		}
	}
	if (HasFlag(flags, RhiClearFlags::Depth)) {
		if (pass_state.has_dsv) {
			command_list_->ClearDepthStencilView(pass_state.dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		}
		else if (pass_state.is_default) {
			ReportUnsupportedOnce("DirectX12RhiDevice::Clear(default_depth_buffer_unavailable)");
		}
	}

	EndFramebufferPass(pass_state);
	CloseAndExecuteCommandList();
}

void DirectX12RhiDevice::PushDebugGroup(std::string_view name)
{
	(void)name;
}

void DirectX12RhiDevice::PopDebugGroup()
{
}

void DirectX12RhiDevice::BindShader(const RhiShaderDesc* shader_desc)
{
	if (shader_desc == nullptr) {
		active_builtin_shader_ = Dx12BuiltinPassShader::MeshColor;
		return;
	}

	if (shader_desc->name == "depth_map"
		|| shader_desc->name == "csm_depth_map"
		|| shader_desc->name == "deferred_depth") {
		active_builtin_shader_ = Dx12BuiltinPassShader::DepthOnly;
		return;
	}
	if (shader_desc->name == "skybox") {
		active_builtin_shader_ = Dx12BuiltinPassShader::Skybox;
		return;
	}

	active_builtin_shader_ = Dx12BuiltinPassShader::MeshColor;
}

void DirectX12RhiDevice::BindTextureUnit(uint32_t unit, RhiTextureHandle texture)
{
	if (unit >= kDx12BuiltinTextureBindingCount) {
		ReportUnsupportedOnce("DirectX12RhiDevice::BindTextureUnit(unit_out_of_range)");
		return;
	}
	if (!texture.IsValid()) {
		bound_textures_.erase(unit);
		return;
	}
	const auto texture_iter = texture_resources_.find(texture.value);
	if (texture_iter == texture_resources_.end()) {
		return;
	}

	if (device_ == nullptr || cbv_srv_uav_heap_.heap == nullptr || texture_iter->second.srv_descriptor_index == kInvalidDescriptorIndex) {
		return;
	}

	device_->CopyDescriptorsSimple(
		1,
		GetCpuDescriptorHandle(cbv_srv_uav_heap_, unit),
		GetCpuDescriptorHandle(cbv_srv_uav_heap_, texture_iter->second.srv_descriptor_index),
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	bound_textures_[unit] = texture;
}

void DirectX12RhiDevice::BindDefaultFramebuffer()
{
	BindDefaultFramebuffer(RhiFramebufferBindTarget::Framebuffer);
}

void DirectX12RhiDevice::BindDefaultFramebuffer(RhiFramebufferBindTarget target)
{
	BindFramebufferState(target, FramebufferBindingState{});
}

void DirectX12RhiDevice::BindFramebuffer(RhiFramebufferBindTarget target, RhiFramebufferHandle framebuffer)
{
	if (!framebuffer.IsValid()) {
		BindDefaultFramebuffer(target);
		return;
	}
	if (framebuffer_resources_.find(framebuffer.value) == framebuffer_resources_.end()) {
		ReportUnsupportedOnce("DirectX12RhiDevice::BindFramebuffer(invalid_framebuffer)");
		return;
	}
	BindFramebufferState(target, FramebufferBindingState{ framebuffer, false });
}

uint32_t DirectX12RhiDevice::GetDefaultFramebuffer() const
{
	return kInvalidDefaultFramebuffer;
}

void DirectX12RhiDevice::ResizePresentSurface(uint32_t width, uint32_t height)
{
	init_config_.present_surface.width = width;
	init_config_.present_surface.height = height;
	if (width == 0 || height == 0) {
		return;
	}
	if (HasOwnedPresentSurface()) {
		ResizeOwnedPresentSurface(width, height);
		return;
	}
	CreatePresentSurfaceResources();
}

void DirectX12RhiDevice::Present()
{
	if (!HasOwnedPresentSurface()) {
		return;
	}

	const HRESULT hr = present_surface_state_.swap_chain->Present(0, 0);
	if (FAILED(hr)) {
		ReportFailure("IDXGISwapChain::Present", hr);
		return;
	}

	present_surface_state_.current_back_buffer_index = present_surface_state_.swap_chain->GetCurrentBackBufferIndex();
}

RhiBufferHandle DirectX12RhiDevice::CreateBuffer(const RhiBufferDesc& desc, const void* initial_data)
{
	if (!initialized_ || device_ == nullptr || desc.size == 0) {
		return {};
	}

	BufferResource buffer_resource{};
	buffer_resource.desc = desc;
	if (!CreateUploadBufferAllocation(GetCommittedBufferSize(desc), desc.size, initial_data, buffer_resource.allocation)) {
		return {};
	}

	const uint64_t handle_value = next_buffer_handle_++;
	buffer_resources_.emplace(handle_value, std::move(buffer_resource));
	return RhiBufferHandle{ handle_value };
}

void DirectX12RhiDevice::BindBuffer(RhiBufferUsage usage, RhiBufferHandle buffer)
{
	if (buffer.value == 0) {
		bound_buffers_.erase(usage);
		return;
	}

	const auto iter = buffer_resources_.find(buffer.value);
	if (iter == buffer_resources_.end()) {
		return;
	}
	if (iter->second.desc.usage != usage) {
		return;
	}
	bound_buffers_[usage] = buffer;
}

void DirectX12RhiDevice::BindUniformBufferBase(uint32_t binding, RhiBufferHandle buffer)
{
	if (buffer.value == 0) {
		uniform_buffer_bindings_.erase(binding);
		return;
	}

	const auto iter = buffer_resources_.find(buffer.value);
	if (!initialized_ || iter == buffer_resources_.end() || iter->second.desc.usage != RhiBufferUsage::Uniform) {
		return;
	}

	auto& buffer_resource = iter->second;
	if (!EnsureUniformBufferDescriptor(buffer_resource)) {
		return;
	}

	uniform_buffer_bindings_[binding] = buffer;
	bound_buffers_[RhiBufferUsage::Uniform] = buffer;
}

void DirectX12RhiDevice::UpdateBuffer(RhiBufferHandle buffer, size_t offset, size_t size, const void* data)
{
	const auto iter = buffer_resources_.find(buffer.value);
	if (!initialized_ || iter == buffer_resources_.end() || data == nullptr || size == 0) {
		return;
	}
	if (offset + size > iter->second.allocation.logical_size) {
		ReportUnsupportedOnce("DirectX12RhiDevice::UpdateBuffer(out_of_range)");
		return;
	}
	WriteUploadBufferAllocation(iter->second.allocation, offset, size, data);
}

void DirectX12RhiDevice::DestroyBuffer(RhiBufferHandle buffer)
{
	const auto iter = buffer_resources_.find(buffer.value);
	if (iter == buffer_resources_.end()) {
		return;
	}

	for (auto binding_iter = uniform_buffer_bindings_.begin(); binding_iter != uniform_buffer_bindings_.end();) {
		if (binding_iter->second.value == buffer.value) {
			binding_iter = uniform_buffer_bindings_.erase(binding_iter);
		}
		else {
			++binding_iter;
		}
	}

	for (auto bound_iter = bound_buffers_.begin(); bound_iter != bound_buffers_.end();) {
		if (bound_iter->second.value == buffer.value) {
			bound_iter = bound_buffers_.erase(bound_iter);
		}
		else {
			++bound_iter;
		}
	}

	if (iter->second.descriptor_index != kInvalidDescriptorIndex) {
		FreeDescriptor(cbv_srv_uav_heap_, iter->second.descriptor_index);
	}
	auto buffer_resource = std::move(iter->second);
	buffer_resources_.erase(iter);
	ReleaseUploadBufferAllocation(buffer_resource.allocation);
}

RhiTextureHandle DirectX12RhiDevice::CreateTexture(const RhiTextureDesc& desc, const void* initial_data)
{
	if (!initialized_ || device_ == nullptr || desc.width <= 0 || desc.height <= 0) {
		return {};
	}

	const bool create_dsv = ToDxgiDepthStencilFormat(desc.format) != DXGI_FORMAT_UNKNOWN;
	RhiTextureHandle texture_handle{};
	if (!CreateFramebufferTextureResource(desc, false, create_dsv, texture_handle)) {
		ReportUnsupportedOnce("DirectX12RhiDevice::CreateTexture");
		return {};
	}

	if (initial_data == nullptr || create_dsv) {
		return texture_handle;
	}

	const auto texture_iter = texture_resources_.find(texture_handle.value);
	if (texture_iter == texture_resources_.end()) {
		DestroyTextureHandle(texture_handle.value);
		return {};
	}

	const size_t pixel_stride = GetTexturePixelStride(texture_iter->second.shader_resource_format);
	const auto subresources = BuildTextureUploadSubresources(desc, pixel_stride, initial_data);
	if (subresources.empty() || !UploadTextureSubresources(texture_iter->second, subresources)) {
		DestroyTextureHandle(texture_handle.value);
		ReportUnsupportedOnce("DirectX12RhiDevice::CreateTexture(upload_failed)");
		return {};
	}
	return texture_handle;
}

void DirectX12RhiDevice::DestroyTexture(RhiTextureHandle texture)
{
	if (!texture.IsValid()) {
		return;
	}
	DestroyTextureHandle(texture.value);
}

RhiTextureHandle DirectX12RhiDevice::LoadTexture2D(std::string_view path, bool repeat)
{
	(void)repeat;
	if (!initialized_ || device_ == nullptr || path.empty()) {
		return {};
	}

	const std::string owned_path(path);
	int width = 0;
	int height = 0;
	int channel_count = 0;
	if (stbi_is_hdr(owned_path.c_str()) != 0) {
		float* data = stbi_loadf(owned_path.c_str(), &width, &height, &channel_count, STBI_rgb_alpha);
		if (data == nullptr) {
			ReportUnsupportedOnce("DirectX12RhiDevice::LoadTexture2D(load_failed)");
			return {};
		}

		const RhiTextureHandle texture = CreateTexture(RhiTextureDesc{
			.dimension = RhiTextureDimension::Texture2D,
			.format = RhiTextureFormat::Rgb32Float,
			.width = width,
			.height = height
		}, data);
		stbi_image_free(data);
		return texture;
	}

	stbi_uc* data = stbi_load(owned_path.c_str(), &width, &height, &channel_count, STBI_rgb_alpha);
	if (data == nullptr) {
		ReportUnsupportedOnce("DirectX12RhiDevice::LoadTexture2D(load_failed)");
		return {};
	}

	const RhiTextureHandle texture = CreateTexture(RhiTextureDesc{
		.dimension = RhiTextureDimension::Texture2D,
		.format = RhiTextureFormat::Rgba8Unorm,
		.width = width,
		.height = height
	}, data);
	stbi_image_free(data);
	return texture;
}

RhiTextureHandle DirectX12RhiDevice::LoadCubemap(const std::vector<std::string_view>& paths)
{
	if (!initialized_ || device_ == nullptr || paths.size() != 6) {
		return {};
	}

	const std::string first_path(paths.front());
	const bool is_hdr = stbi_is_hdr(first_path.c_str()) != 0;
	int width = 0;
	int height = 0;
	int channel_count = 0;

	if (is_hdr) {
		std::vector<float> face_pixels;
		for (size_t face_index = 0; face_index < paths.size(); ++face_index) {
			const std::string face_path(paths[face_index]);
			int face_width = 0;
			int face_height = 0;
			int face_channels = 0;
			float* face_data = stbi_loadf(face_path.c_str(), &face_width, &face_height, &face_channels, STBI_rgb_alpha);
			if (face_data == nullptr) {
				ReportUnsupportedOnce("DirectX12RhiDevice::LoadCubemap(load_failed)");
				return {};
			}

			if (width == 0 || height == 0) {
				width = face_width;
				height = face_height;
				channel_count = face_channels;
				face_pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4u * paths.size());
			}
			else if (face_width != width || face_height != height || face_channels != channel_count) {
				stbi_image_free(face_data);
				ReportUnsupportedOnce("DirectX12RhiDevice::LoadCubemap(face_mismatch)");
				return {};
			}

			const size_t face_pixel_count = static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
			std::memcpy(face_pixels.data() + face_pixel_count * face_index, face_data, face_pixel_count * sizeof(float));
			stbi_image_free(face_data);
		}

		return CreateTexture(RhiTextureDesc{
			.dimension = RhiTextureDimension::TextureCube,
			.format = RhiTextureFormat::Rgb32Float,
			.width = width,
			.height = height
		}, face_pixels.data());
	}

	std::vector<stbi_uc> face_pixels;
	for (size_t face_index = 0; face_index < paths.size(); ++face_index) {
		const std::string face_path(paths[face_index]);
		int face_width = 0;
		int face_height = 0;
		int face_channels = 0;
		stbi_uc* face_data = stbi_load(face_path.c_str(), &face_width, &face_height, &face_channels, STBI_rgb_alpha);
		if (face_data == nullptr) {
			ReportUnsupportedOnce("DirectX12RhiDevice::LoadCubemap(load_failed)");
			return {};
		}

		if (width == 0 || height == 0) {
			width = face_width;
			height = face_height;
			channel_count = face_channels;
			face_pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4u * paths.size());
		}
		else if (face_width != width || face_height != height || face_channels != channel_count) {
			stbi_image_free(face_data);
			ReportUnsupportedOnce("DirectX12RhiDevice::LoadCubemap(face_mismatch)");
			return {};
		}

		const size_t face_pixel_count = static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
		std::memcpy(face_pixels.data() + face_pixel_count * face_index, face_data, face_pixel_count * sizeof(stbi_uc));
		stbi_image_free(face_data);
	}

	return CreateTexture(RhiTextureDesc{
		.dimension = RhiTextureDimension::TextureCube,
		.format = RhiTextureFormat::Rgba8Unorm,
		.width = width,
		.height = height
	}, face_pixels.data());
}

RhiFramebufferHandle DirectX12RhiDevice::CreatePickingFramebuffer(int width, int height)
{
	return CreateFramebuffer(RhiFramebufferCreateDesc{
		.width = width,
		.height = height,
		.color_attachments = { RhiTextureFormat::Rgb32Float }
	});
}

RhiFramebufferHandle DirectX12RhiDevice::CreateFramebuffer(const RhiFramebufferCreateDesc& desc)
{
	if (!initialized_ || device_ == nullptr || desc.width <= 0 || desc.height <= 0) {
		return {};
	}

	FramebufferResource framebuffer_resource{};
	framebuffer_resource.desc = desc;

	if (!desc.color_attachments.empty()) {
		framebuffer_resource.textures.reserve(desc.color_attachments.size());
		framebuffer_resource.owns_textures.reserve(desc.color_attachments.size());
		for (const auto format : desc.color_attachments) {
			RhiTextureHandle texture_handle{};
			if (!CreateFramebufferTextureResource(RhiTextureDesc{
				.dimension = RhiTextureDimension::Texture2D,
				.format = format,
				.width = desc.width,
				.height = desc.height
			}, true, false, texture_handle)) {
				ReleaseFramebufferTextures(framebuffer_resource);
				return {};
			}
			framebuffer_resource.textures.push_back(texture_handle);
			framebuffer_resource.owns_textures.push_back(true);
		}

		RhiTextureHandle depth_texture{};
		if (!CreateFramebufferTextureResource(RhiTextureDesc{
			.dimension = RhiTextureDimension::Texture2D,
			.format = RhiTextureFormat::Depth24Stencil8,
			.width = desc.width,
			.height = desc.height
		}, false, true, depth_texture)) {
			ReleaseFramebufferTextures(framebuffer_resource);
			return {};
		}
		framebuffer_resource.depth_stencil_texture = depth_texture;
		framebuffer_resource.owns_depth_stencil_texture = true;

		const uint64_t framebuffer_handle_value = next_framebuffer_handle_++;
		framebuffer_resources_.emplace(framebuffer_handle_value, std::move(framebuffer_resource));
		return RhiFramebufferHandle{ framebuffer_handle_value };
	}

	const RhiTextureDesc attachment_desc{
		.dimension = GetFramebufferTextureDimension(desc),
		.format = GetFramebufferAttachmentFormat(desc),
		.width = desc.width,
		.height = desc.height,
		.layers = desc.layers > 0 ? desc.layers : 1
	};
	const bool is_depth_attachment = desc.attachment == RhiFramebufferAttachment::Depth;
	RhiTextureHandle primary_texture{};
	if (!CreateFramebufferTextureResource(attachment_desc, !is_depth_attachment, is_depth_attachment, primary_texture)) {
		return {};
	}

	framebuffer_resource.textures.push_back(primary_texture);
	framebuffer_resource.owns_textures.push_back(true);
	if (is_depth_attachment) {
		framebuffer_resource.depth_stencil_texture = primary_texture;
		framebuffer_resource.owns_depth_stencil_texture = true;
	}
	else {
		RhiTextureHandle depth_texture{};
		if (!CreateFramebufferTextureResource(RhiTextureDesc{
			.dimension = RhiTextureDimension::Texture2D,
			.format = RhiTextureFormat::Depth24Stencil8,
			.width = desc.width,
			.height = desc.height
		}, false, true, depth_texture)) {
			ReleaseFramebufferTextures(framebuffer_resource);
			return {};
		}
		framebuffer_resource.depth_stencil_texture = depth_texture;
		framebuffer_resource.owns_depth_stencil_texture = true;
	}

	const uint64_t framebuffer_handle_value = next_framebuffer_handle_++;
	framebuffer_resources_.emplace(framebuffer_handle_value, std::move(framebuffer_resource));
	return RhiFramebufferHandle{ framebuffer_handle_value };
}

void DirectX12RhiDevice::DestroyFramebuffer(RhiFramebufferHandle framebuffer)
{
	if (!framebuffer.IsValid()) {
		return;
	}

	const auto iter = framebuffer_resources_.find(framebuffer.value);
	if (iter != framebuffer_resources_.end()) {
		ReleaseFramebufferTextures(iter->second);
		framebuffer_resources_.erase(iter);
	}

	if (!draw_framebuffer_binding_.is_default && draw_framebuffer_binding_.framebuffer.value == framebuffer.value) {
		draw_framebuffer_binding_ = {};
	}
	if (!read_framebuffer_binding_.is_default && read_framebuffer_binding_.framebuffer.value == framebuffer.value) {
		read_framebuffer_binding_ = {};
	}
}

RhiTextureHandle DirectX12RhiDevice::GetFramebufferTexture(RhiFramebufferHandle framebuffer) const
{
	return GetFramebufferColorTexture(framebuffer, 0);
}

RhiTextureHandle DirectX12RhiDevice::GetFramebufferColorTexture(RhiFramebufferHandle framebuffer, uint32_t index) const
{
	const auto iter = framebuffer_resources_.find(framebuffer.value);
	if (iter == framebuffer_resources_.end() || index >= iter->second.textures.size()) {
		return {};
	}
	return iter->second.textures[index];
}

RhiTextureHandle DirectX12RhiDevice::TakeFramebufferTexture(RhiFramebufferHandle framebuffer)
{
	const auto iter = framebuffer_resources_.find(framebuffer.value);
	if (iter == framebuffer_resources_.end() || iter->second.textures.empty()) {
		return {};
	}

	const RhiTextureHandle texture = iter->second.textures.front();
	iter->second.textures.front() = {};
	if (!iter->second.owns_textures.empty()) {
		iter->second.owns_textures.front() = false;
	}
	if (iter->second.depth_stencil_texture.value == texture.value) {
		iter->second.depth_stencil_texture = {};
		iter->second.owns_depth_stencil_texture = false;
	}
	return texture;
}

RhiTextureHandle DirectX12RhiDevice::ReallocateFramebufferTexture(RhiFramebufferHandle framebuffer, const RhiFramebufferCreateDesc& desc)
{
	const auto iter = framebuffer_resources_.find(framebuffer.value);
	if (iter == framebuffer_resources_.end() || iter->second.textures.empty()) {
		return {};
	}

	const RhiTextureHandle old_texture = iter->second.textures.front();
	const auto replacement_framebuffer = CreateFramebuffer(desc);
	const auto replacement_iter = framebuffer_resources_.find(replacement_framebuffer.value);
	if (!replacement_framebuffer.IsValid() || replacement_iter == framebuffer_resources_.end() || replacement_iter->second.textures.empty()) {
		return old_texture;
	}

	ReleaseFramebufferTextures(iter->second, old_texture.value);
	iter->second = std::move(replacement_iter->second);
	framebuffer_resources_.erase(replacement_iter);
	return old_texture;
}

void DirectX12RhiDevice::ResizeFramebufferRenderbuffer(RhiFramebufferHandle framebuffer, int width, int height)
{
	const auto iter = framebuffer_resources_.find(framebuffer.value);
	if (iter == framebuffer_resources_.end()
		|| !iter->second.depth_stencil_texture.IsValid()
		|| iter->second.depth_stencil_texture.value == GetFramebufferTexture(framebuffer).value
		|| width <= 0
		|| height <= 0) {
		return;
	}

	const auto texture_iter = texture_resources_.find(iter->second.depth_stencil_texture.value);
	if (texture_iter == texture_resources_.end()) {
		return;
	}

	const RhiTextureDesc resized_desc{
		.dimension = texture_iter->second.desc.dimension,
		.format = texture_iter->second.desc.format,
		.width = width,
		.height = height,
		.layers = texture_iter->second.desc.layers,
		.mip_levels = texture_iter->second.desc.mip_levels
	};

	RhiTextureHandle replacement{};
	if (!CreateFramebufferTextureResource(resized_desc, false, true, replacement)) {
		return;
	}

	DestroyTextureHandle(iter->second.depth_stencil_texture.value);
	iter->second.depth_stencil_texture = replacement;
	iter->second.owns_depth_stencil_texture = true;
}

void DirectX12RhiDevice::ReadFramebufferColorPixel(RhiFramebufferHandle framebuffer, uint32_t x, uint32_t y, float* rgb)
{
	if (rgb != nullptr) {
		rgb[0] = 0.0f;
		rgb[1] = 0.0f;
		rgb[2] = 0.0f;
	}
	if (rgb == nullptr) {
		return;
	}

	const RhiTextureHandle texture_handle = GetFramebufferTexture(framebuffer);
	const auto iter = texture_resources_.find(texture_handle.value);
	if (!texture_handle.IsValid() || iter == texture_resources_.end() || iter->second.resource == nullptr || iter->second.is_depth_stencil) {
		return;
	}
	if (x >= static_cast<uint32_t>(std::max(iter->second.desc.width, 0)) || y >= static_cast<uint32_t>(std::max(iter->second.desc.height, 0))) {
		return;
	}

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
	UINT row_count = 0;
	UINT64 row_size_bytes = 0;
	UINT64 total_bytes = 0;
	if (!EnsureTextureReadbackResource(iter->second, footprint, row_count, row_size_bytes, total_bytes) || !ResetCommandListForRecording()) {
		return;
	}
	(void)row_count;
	(void)row_size_bytes;

	const D3D12_RESOURCE_STATES previous_state = iter->second.current_state;
	if (!TransitionTextureResource(iter->second, D3D12_RESOURCE_STATE_COPY_SOURCE)) {
		return;
	}

	D3D12_TEXTURE_COPY_LOCATION source_location{};
	source_location.pResource = iter->second.resource.Get();
	source_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	source_location.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION destination_location{};
	destination_location.pResource = iter->second.readback_resource.Get();
	destination_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	destination_location.PlacedFootprint = footprint;

	command_list_->CopyTextureRegion(&destination_location, 0, 0, 0, &source_location, nullptr);
	TransitionTextureResource(iter->second, previous_state);
	if (!CloseAndExecuteCommandList()) {
		return;
	}

	void* mapped_data = nullptr;
	const D3D12_RANGE read_range{ 0, static_cast<SIZE_T>(total_bytes) };
	if (FAILED(iter->second.readback_resource->Map(0, &read_range, &mapped_data)) || mapped_data == nullptr) {
		return;
	}

	const auto* const bytes = static_cast<const std::byte*>(mapped_data)
		+ footprint.Offset
		+ static_cast<size_t>(y) * footprint.Footprint.RowPitch
		+ static_cast<size_t>(x) * GetTexturePixelStride(iter->second.shader_resource_format);

	switch (iter->second.shader_resource_format) {
	case DXGI_FORMAT_R32G32B32A32_FLOAT: {
		const auto* const floats = reinterpret_cast<const float*>(bytes);
		rgb[0] = floats[0];
		rgb[1] = floats[1];
		rgb[2] = floats[2];
		break;
	}
	case DXGI_FORMAT_R16G16B16A16_FLOAT: {
		const auto* const halves = reinterpret_cast<const uint16_t*>(bytes);
		rgb[0] = HalfToFloat(halves[0]);
		rgb[1] = HalfToFloat(halves[1]);
		rgb[2] = HalfToFloat(halves[2]);
		break;
	}
	case DXGI_FORMAT_R16G16_FLOAT: {
		const auto* const halves = reinterpret_cast<const uint16_t*>(bytes);
		rgb[0] = HalfToFloat(halves[0]);
		rgb[1] = HalfToFloat(halves[1]);
		rgb[2] = 0.0f;
		break;
	}
	default:
		break;
	}

	const D3D12_RANGE empty_range{};
	iter->second.readback_resource->Unmap(0, &empty_range);
}

RhiMeshHandle DirectX12RhiDevice::CreateMesh(const RhiMeshDesc& desc)
{
	if (!initialized_ || device_ == nullptr || desc.vertex_data == nullptr || desc.vertex_data_size == 0 || desc.vertex_stride == 0) {
		return {};
	}

	MeshResource mesh_resource{};
	mesh_resource.vertex_stride = desc.vertex_stride;
	mesh_resource.vertex_count = desc.vertex_count;
	mesh_resource.index_count = desc.index_count;
	mesh_resource.vertex_layout = desc.vertex_layout;

	if (!CreateUploadBufferAllocation(desc.vertex_data_size, desc.vertex_data_size, desc.vertex_data, mesh_resource.vertex_buffer)) {
		return {};
	}

	mesh_resource.vertex_buffer_view.BufferLocation = mesh_resource.vertex_buffer.gpu_address;
	mesh_resource.vertex_buffer_view.SizeInBytes = static_cast<UINT>(mesh_resource.vertex_buffer.logical_size);
	mesh_resource.vertex_buffer_view.StrideInBytes = static_cast<UINT>(desc.vertex_stride);

	if (desc.index_data != nullptr && desc.index_data_size > 0) {
		if (!CreateUploadBufferAllocation(desc.index_data_size, desc.index_data_size, desc.index_data, mesh_resource.index_buffer)) {
			ReleaseUploadBufferAllocation(mesh_resource.vertex_buffer);
			return {};
		}
		mesh_resource.has_index_buffer = true;
		mesh_resource.index_buffer_view.BufferLocation = mesh_resource.index_buffer.gpu_address;
		mesh_resource.index_buffer_view.SizeInBytes = static_cast<UINT>(mesh_resource.index_buffer.logical_size);
		mesh_resource.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
	}

	const uint64_t handle_value = next_mesh_handle_++;
	mesh_resources_.emplace(handle_value, std::move(mesh_resource));
	return RhiMeshHandle{ handle_value };
}

void DirectX12RhiDevice::UpdateMeshVertexData(RhiMeshHandle mesh, size_t offset, size_t size, const void* data)
{
	const auto iter = mesh_resources_.find(mesh.value);
	if (!initialized_ || iter == mesh_resources_.end() || data == nullptr || size == 0) {
		return;
	}
	if (offset + size > iter->second.vertex_buffer.logical_size) {
		ReportUnsupportedOnce("DirectX12RhiDevice::UpdateMeshVertexData(out_of_range)");
		return;
	}
	WriteUploadBufferAllocation(iter->second.vertex_buffer, offset, size, data);
}

void DirectX12RhiDevice::DestroyMesh(RhiMeshHandle mesh)
{
	const auto iter = mesh_resources_.find(mesh.value);
	if (iter == mesh_resources_.end()) {
		return;
	}

	auto mesh_resource = std::move(iter->second);
	mesh_resources_.erase(iter);
	ReleaseUploadBufferAllocation(mesh_resource.vertex_buffer);
	ReleaseUploadBufferAllocation(mesh_resource.index_buffer);
}

void DirectX12RhiDevice::DrawMesh(RhiMeshHandle mesh, RhiPrimitiveTopology topology, uint32_t index_count)
{
	const auto iter = mesh_resources_.find(mesh.value);
	if (!initialized_ || iter == mesh_resources_.end()) {
		return;
	}

	const Dx12BuiltinPassShader resolved_shader = ResolveBuiltinPassShader(iter->second.vertex_layout);
	if (resolved_shader == Dx12BuiltinPassShader::Skybox && !GetBoundTextureSrvDescriptorIndex(kDx12BuiltinCubemapBinding).has_value()) {
		ReportUnsupportedOnce("DirectX12RhiDevice::DrawMesh(missing_skybox_cubemap_binding)");
		return;
	}

	const auto& draw_binding = GetFramebufferBindingState(RhiFramebufferBindTarget::Draw);
	ID3D12PipelineState* const pipeline_state = GetBuiltinMeshPipelineState(draw_binding, iter->second.vertex_layout, topology);
	if (pipeline_state == nullptr || builtin_mesh_pipeline_ == nullptr || builtin_mesh_pipeline_->GetRootSignature() == nullptr) {
		return;
	}

	const auto matrices_buffer_address = GetBoundUniformBufferGpuAddress(0);
	if (!matrices_buffer_address.has_value()) {
		ReportUnsupportedOnce("DirectX12RhiDevice::DrawMesh(missing_matrices_uniform_buffer)");
		return;
	}

	const bool use_index_buffer = iter->second.has_index_buffer;
	uint32_t draw_count = index_count;
	if (use_index_buffer) {
		const uint32_t mesh_index_count = static_cast<uint32_t>(iter->second.index_count);
		draw_count = draw_count != 0 ? draw_count : mesh_index_count;
		if (draw_count > mesh_index_count) {
			ReportUnsupportedOnce("DirectX12RhiDevice::DrawMesh(index_count_out_of_range)");
			draw_count = mesh_index_count;
		}
	}
	else {
		const uint32_t mesh_vertex_count = static_cast<uint32_t>(iter->second.vertex_count);
		draw_count = draw_count != 0 ? draw_count : mesh_vertex_count;
		if (draw_count > mesh_vertex_count) {
			ReportUnsupportedOnce("DirectX12RhiDevice::DrawMesh(vertex_count_out_of_range)");
			draw_count = mesh_vertex_count;
		}
	}
	if (draw_count == 0) {
		return;
	}

	if (!ResetCommandListForRecording()) {
		return;
	}

	FramebufferPassState pass_state{};
	if (!BeginFramebufferPass(draw_binding, pass_state)) {
		return;
	}

	std::vector<std::pair<TextureResource*, D3D12_RESOURCE_STATES>> sampled_textures;
	sampled_textures.reserve(2);
	auto transition_bound_texture = [this, &sampled_textures](uint32_t unit) {
		const auto bound_texture = bound_textures_.find(unit);
		if (bound_texture == bound_textures_.end()) {
			return true;
		}

		const auto texture_iter = texture_resources_.find(bound_texture->second.value);
		if (texture_iter == texture_resources_.end()) {
			return false;
		}

		TextureResource& texture_resource = texture_iter->second;
		for (const auto& [tracked_texture, _] : sampled_textures) {
			if (tracked_texture == &texture_resource) {
				return true;
			}
		}

		sampled_textures.emplace_back(&texture_resource, texture_resource.current_state);
		return TransitionTextureResource(texture_resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	};

	bool ready_to_draw = true;
	if (resolved_shader == Dx12BuiltinPassShader::MeshTextured2D) {
		ready_to_draw = transition_bound_texture(kDx12BuiltinTexture2DBinding);
	}
	else if (resolved_shader == Dx12BuiltinPassShader::Skybox) {
		ready_to_draw = transition_bound_texture(kDx12BuiltinCubemapBinding);
	}
	if (!ready_to_draw) {
		for (auto it = sampled_textures.rbegin(); it != sampled_textures.rend(); ++it) {
			TransitionTextureResource(*it->first, it->second);
		}
		EndFramebufferPass(pass_state);
		return;
	}

	command_list_->SetGraphicsRootSignature(builtin_mesh_pipeline_->GetRootSignature());
	command_list_->SetPipelineState(pipeline_state);
	command_list_->IASetPrimitiveTopology(ToD3DPrimitiveTopology(topology));
	command_list_->IASetVertexBuffers(0, 1, &iter->second.vertex_buffer_view);
	if (use_index_buffer) {
		command_list_->IASetIndexBuffer(&iter->second.index_buffer_view);
	}
	command_list_->SetGraphicsRootConstantBufferView(0, *matrices_buffer_address);
	command_list_->SetGraphicsRootDescriptorTable(1, GetGpuDescriptorHandle(cbv_srv_uav_heap_, 0));
	if (use_index_buffer) {
		command_list_->DrawIndexedInstanced(draw_count, 1, 0, 0, 0);
	}
	else {
		command_list_->DrawInstanced(draw_count, 1, 0, 0);
	}

	for (auto it = sampled_textures.rbegin(); it != sampled_textures.rend(); ++it) {
		TransitionTextureResource(*it->first, it->second);
	}

	EndFramebufferPass(pass_state);
	CloseAndExecuteCommandList();
}

DXGI_GPU_PREFERENCE DirectX12RhiDevice::ToDxgiGpuPreference(RhiAdapterPreference preference)
{
	switch (preference) {
	case RhiAdapterPreference::HighPerformance:
		return DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
	case RhiAdapterPreference::LowPower:
		return DXGI_GPU_PREFERENCE_MINIMUM_POWER;
	case RhiAdapterPreference::Default:
		break;
	}
	return DXGI_GPU_PREFERENCE_UNSPECIFIED;
}

size_t DirectX12RhiDevice::GetCommittedBufferSize(const RhiBufferDesc& desc)
{
	if (desc.usage != RhiBufferUsage::Uniform) {
		return desc.size;
	}
	const size_t alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
	return ((desc.size + alignment - 1) / alignment) * alignment;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE DirectX12RhiDevice::ToD3DPrimitiveTopologyType(RhiPrimitiveTopology topology)
{
	switch (topology) {
	case RhiPrimitiveTopology::Points:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	case RhiPrimitiveTopology::Lines:
	case RhiPrimitiveTopology::LineStrip:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case RhiPrimitiveTopology::Triangles:
	default:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}
}

D3D12_COMPARISON_FUNC DirectX12RhiDevice::ToD3DComparisonFunc(RhiDepthFunc func)
{
	switch (func) {
	case RhiDepthFunc::LessEqual:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case RhiDepthFunc::Less:
	default:
		return D3D12_COMPARISON_FUNC_LESS;
	}
}

void DirectX12RhiDevice::EnableDebugLayerIfRequested()
{
	if (!init_config_.enable_debug_layer && !init_config_.enable_validation) {
		return;
	}

	Microsoft::WRL::ComPtr<ID3D12Debug> debug_controller;
	const HRESULT debug_hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
	if (FAILED(debug_hr)) {
		ReportFailure("D3D12GetDebugInterface", debug_hr);
		return;
	}

	debug_controller->EnableDebugLayer();

	if (init_config_.enable_validation) {
		Microsoft::WRL::ComPtr<ID3D12Debug1> debug_controller_1;
		if (SUCCEEDED(debug_controller.As(&debug_controller_1))) {
			debug_controller_1->SetEnableGPUBasedValidation(true);
		}
	}
}

bool DirectX12RhiDevice::CreateFactory()
{
	UINT factory_flags = 0;
	if (init_config_.enable_debug_layer || init_config_.enable_validation) {
		factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
	}

	HRESULT hr = CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory_));
	if (FAILED(hr) && factory_flags != 0) {
		hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory_));
	}
	if (FAILED(hr)) {
		ReportFailure("CreateDXGIFactory2", hr);
		return false;
	}
	return true;
}

bool DirectX12RhiDevice::CreateAdapter()
{
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter = FindHardwareAdapter();
	if (adapter == nullptr) {
		adapter = FindWarpAdapter();
	}
	if (adapter == nullptr) {
		std::printf("DirectX12RhiDevice: no compatible DXGI adapter was found.\n");
		return false;
	}

	const HRESULT hr = adapter.As(&adapter_);
	if (FAILED(hr)) {
		ReportFailure("IDXGIAdapter1::QueryInterface(IDXGIAdapter4)", hr);
		return false;
	}
	return true;
}

bool DirectX12RhiDevice::CreateDevice()
{
	const HRESULT hr = D3D12CreateDevice(adapter_.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_));
	if (FAILED(hr)) {
		ReportFailure("D3D12CreateDevice", hr);
		return false;
	}
	return true;
}

bool DirectX12RhiDevice::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC queue_desc{};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.NodeMask = 0;

	const HRESULT hr = device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue_));
	if (FAILED(hr)) {
		ReportFailure("ID3D12Device::CreateCommandQueue", hr);
		return false;
	}
	return true;
}

bool DirectX12RhiDevice::CreateCommandObjects()
{
	const HRESULT allocator_hr = device_->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&command_allocator_));
	if (FAILED(allocator_hr)) {
		ReportFailure("ID3D12Device::CreateCommandAllocator", allocator_hr);
		return false;
	}

	const HRESULT list_hr = device_->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		command_allocator_.Get(),
		nullptr,
		IID_PPV_ARGS(&command_list_));
	if (FAILED(list_hr)) {
		ReportFailure("ID3D12Device::CreateCommandList", list_hr);
		return false;
	}

	const HRESULT close_hr = command_list_->Close();
	if (FAILED(close_hr)) {
		ReportFailure("ID3D12GraphicsCommandList::Close", close_hr);
		return false;
	}

	command_list_ready_ = false;
	return true;
}

bool DirectX12RhiDevice::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_uav_heap_desc{};
	cbv_srv_uav_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbv_srv_uav_heap_desc.NumDescriptors = kDefaultCbvHeapCapacity;
	cbv_srv_uav_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbv_srv_uav_heap_desc.NodeMask = 0;

	const HRESULT cbv_srv_uav_hr = device_->CreateDescriptorHeap(&cbv_srv_uav_heap_desc, IID_PPV_ARGS(&cbv_srv_uav_heap_.heap));
	if (FAILED(cbv_srv_uav_hr)) {
		ReportFailure("ID3D12Device::CreateDescriptorHeap(CBV_SRV_UAV)", cbv_srv_uav_hr);
		return false;
	}

	cbv_srv_uav_heap_.descriptor_size = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	cbv_srv_uav_heap_.capacity = cbv_srv_uav_heap_desc.NumDescriptors;
	cbv_srv_uav_heap_.next_index = kDx12BuiltinTextureBindingCount;
	cbv_srv_uav_heap_.free_indices.clear();

	D3D12_DESCRIPTOR_HEAP_DESC offscreen_rtv_heap_desc{};
	offscreen_rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	offscreen_rtv_heap_desc.NumDescriptors = kDefaultOffscreenRtvHeapCapacity;
	offscreen_rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	offscreen_rtv_heap_desc.NodeMask = 0;

	const HRESULT rtv_hr = device_->CreateDescriptorHeap(&offscreen_rtv_heap_desc, IID_PPV_ARGS(&offscreen_rtv_heap_.heap));
	if (FAILED(rtv_hr)) {
		ReportFailure("ID3D12Device::CreateDescriptorHeap(RTV)", rtv_hr);
		return false;
	}

	offscreen_rtv_heap_.descriptor_size = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	offscreen_rtv_heap_.capacity = offscreen_rtv_heap_desc.NumDescriptors;
	offscreen_rtv_heap_.next_index = 0;
	offscreen_rtv_heap_.free_indices.clear();

	D3D12_DESCRIPTOR_HEAP_DESC offscreen_dsv_heap_desc{};
	offscreen_dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	offscreen_dsv_heap_desc.NumDescriptors = kDefaultOffscreenDsvHeapCapacity;
	offscreen_dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	offscreen_dsv_heap_desc.NodeMask = 0;

	const HRESULT dsv_hr = device_->CreateDescriptorHeap(&offscreen_dsv_heap_desc, IID_PPV_ARGS(&offscreen_dsv_heap_.heap));
	if (FAILED(dsv_hr)) {
		ReportFailure("ID3D12Device::CreateDescriptorHeap(DSV)", dsv_hr);
		return false;
	}

	offscreen_dsv_heap_.descriptor_size = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	offscreen_dsv_heap_.capacity = offscreen_dsv_heap_desc.NumDescriptors;
	offscreen_dsv_heap_.next_index = 0;
	offscreen_dsv_heap_.free_indices.clear();
	return true;
}

bool DirectX12RhiDevice::CreateFence()
{
	const HRESULT hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	if (FAILED(hr)) {
		ReportFailure("ID3D12Device::CreateFence", hr);
		return false;
	}

	fence_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fence_event_ == nullptr) {
		const HRESULT last_error = HRESULT_FROM_WIN32(GetLastError());
		ReportFailure("CreateEvent(fence)", last_error);
		return false;
	}
	return true;
}

bool DirectX12RhiDevice::CreatePresentSurfaceResources()
{
	present_surface_state_ = {};
	if (!init_config_.present_surface.HasNativeSurface()
		|| init_config_.present_surface.default_render_target_ownership != RhiDefaultRenderTargetOwnership::Backend) {
		return true;
	}

	if (init_config_.present_surface.native_surface.type != RhiNativeSurfaceType::Win32Hwnd) {
		ReportUnsupportedOnce("DirectX12RhiDevice::InitializeForSurface(unsupported_native_surface)");
		return false;
	}

	HWND hwnd = reinterpret_cast<HWND>(init_config_.present_surface.native_surface.handle);
	if (!::IsWindow(hwnd)) {
		std::printf("DirectX12RhiDevice: invalid HWND for DX12 present surface.\n");
		return false;
	}

	const uint32_t width = std::max<uint32_t>(init_config_.present_surface.width, 1u);
	const uint32_t height = std::max<uint32_t>(init_config_.present_surface.height, 1u);
	return CreateSwapChainForPresentSurface(hwnd, width, height) && CreateSwapChainBackBuffers();
}

bool DirectX12RhiDevice::CreateBuiltinMeshPipelineResources()
{
	if (device_ == nullptr) {
		return false;
	}

	builtin_mesh_pipeline_ = std::make_unique<Dx12BuiltinMeshPipelineLibrary>();
	if (!builtin_mesh_pipeline_->Initialize(device_.Get())) {
		builtin_mesh_pipeline_.reset();
		return false;
	}
	return true;
}

bool DirectX12RhiDevice::CreateTextureResource(const RhiTextureDesc& desc,
	DXGI_FORMAT resource_format,
	DXGI_FORMAT shader_resource_format,
	DXGI_FORMAT render_target_format,
	DXGI_FORMAT depth_stencil_format,
	bool create_rtv,
	bool create_dsv,
	RhiTextureHandle& texture_handle)
{
	texture_handle = {};
	if (device_ == nullptr || desc.width <= 0 || desc.height <= 0) {
		return false;
	}

	TextureResource texture_resource{};
	texture_resource.desc = desc;
	texture_resource.resource_format = resource_format;
	texture_resource.shader_resource_format = shader_resource_format;
	texture_resource.render_target_format = create_rtv ? render_target_format : DXGI_FORMAT_UNKNOWN;
	texture_resource.depth_stencil_format = create_dsv ? depth_stencil_format : DXGI_FORMAT_UNKNOWN;
	texture_resource.is_depth_stencil = create_dsv;

	if (texture_resource.resource_format == DXGI_FORMAT_UNKNOWN
		|| (create_rtv && texture_resource.render_target_format == DXGI_FORMAT_UNKNOWN)
		|| (create_dsv && texture_resource.depth_stencil_format == DXGI_FORMAT_UNKNOWN)) {
		return false;
	}

	D3D12_RESOURCE_DESC resource_desc{};
	resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resource_desc.Alignment = 0;
	resource_desc.Width = static_cast<UINT64>(std::max(desc.width, 1));
	resource_desc.Height = static_cast<UINT>(std::max(desc.height, 1));
	resource_desc.DepthOrArraySize = static_cast<UINT16>(desc.dimension == RhiTextureDimension::TextureCube ? 6 : std::max(desc.layers, 1));
	resource_desc.MipLevels = static_cast<UINT16>(std::max(desc.mip_levels, 1));
	resource_desc.Format = texture_resource.resource_format;
	resource_desc.SampleDesc.Count = 1;
	resource_desc.SampleDesc.Quality = 0;
	resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	if (create_rtv) {
		resource_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if (create_dsv) {
		resource_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}

	D3D12_HEAP_PROPERTIES heap_properties{};
	heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 1;
	heap_properties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE clear_value{};
	D3D12_CLEAR_VALUE* clear_value_ptr = nullptr;
	if (create_dsv) {
		clear_value.Format = texture_resource.depth_stencil_format;
		clear_value.DepthStencil.Depth = 1.0f;
		clear_value.DepthStencil.Stencil = 0;
		clear_value_ptr = &clear_value;
	}
	else if (create_rtv) {
		clear_value.Format = texture_resource.render_target_format;
		clear_value.Color[0] = 0.0f;
		clear_value.Color[1] = 0.0f;
		clear_value.Color[2] = 0.0f;
		clear_value.Color[3] = 0.0f;
		clear_value_ptr = &clear_value;
	}

	const HRESULT create_resource_hr = device_->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&resource_desc,
		D3D12_RESOURCE_STATE_COMMON,
		clear_value_ptr,
		IID_PPV_ARGS(&texture_resource.resource));
	if (FAILED(create_resource_hr)) {
		ReportFailure("ID3D12Device::CreateCommittedResource(texture)", create_resource_hr);
		return false;
	}

	texture_resource.srv_descriptor_index = AllocateDescriptor(cbv_srv_uav_heap_);
	if (texture_resource.srv_descriptor_index == kInvalidDescriptorIndex) {
		ReleaseTextureResource(texture_resource);
		return false;
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
	srv_desc.Format = texture_resource.shader_resource_format;
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (desc.dimension) {
	case RhiTextureDimension::Texture2D:
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MostDetailedMip = 0;
		srv_desc.Texture2D.MipLevels = static_cast<UINT>(std::max(desc.mip_levels, 1));
		srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RhiTextureDimension::Texture2DArray:
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srv_desc.Texture2DArray.MostDetailedMip = 0;
		srv_desc.Texture2DArray.MipLevels = static_cast<UINT>(std::max(desc.mip_levels, 1));
		srv_desc.Texture2DArray.FirstArraySlice = 0;
		srv_desc.Texture2DArray.ArraySize = static_cast<UINT>(std::max(desc.layers, 1));
		srv_desc.Texture2DArray.PlaneSlice = 0;
		srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		break;
	case RhiTextureDimension::TextureCube:
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srv_desc.TextureCube.MostDetailedMip = 0;
		srv_desc.TextureCube.MipLevels = static_cast<UINT>(std::max(desc.mip_levels, 1));
		srv_desc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	}
	device_->CreateShaderResourceView(
		texture_resource.resource.Get(),
		&srv_desc,
		GetCpuDescriptorHandle(cbv_srv_uav_heap_, texture_resource.srv_descriptor_index));

	if (create_rtv) {
		texture_resource.rtv_descriptor_index = AllocateDescriptor(offscreen_rtv_heap_);
		if (texture_resource.rtv_descriptor_index == kInvalidDescriptorIndex) {
			ReleaseTextureResource(texture_resource);
			return false;
		}

		D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{};
		rtv_desc.Format = texture_resource.render_target_format;
		if (desc.dimension == RhiTextureDimension::Texture2D) {
			rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtv_desc.Texture2D.MipSlice = 0;
			rtv_desc.Texture2D.PlaneSlice = 0;
		}
		else {
			rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtv_desc.Texture2DArray.MipSlice = 0;
			rtv_desc.Texture2DArray.FirstArraySlice = 0;
			rtv_desc.Texture2DArray.ArraySize = resource_desc.DepthOrArraySize;
			rtv_desc.Texture2DArray.PlaneSlice = 0;
		}
		device_->CreateRenderTargetView(
			texture_resource.resource.Get(),
			&rtv_desc,
			GetCpuDescriptorHandle(offscreen_rtv_heap_, texture_resource.rtv_descriptor_index));
	}

	if (create_dsv) {
		texture_resource.dsv_descriptor_index = AllocateDescriptor(offscreen_dsv_heap_);
		if (texture_resource.dsv_descriptor_index == kInvalidDescriptorIndex) {
			ReleaseTextureResource(texture_resource);
			return false;
		}

		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
		dsv_desc.Format = texture_resource.depth_stencil_format;
		dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
		if (desc.dimension == RhiTextureDimension::Texture2D) {
			dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv_desc.Texture2D.MipSlice = 0;
		}
		else {
			dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			dsv_desc.Texture2DArray.MipSlice = 0;
			dsv_desc.Texture2DArray.FirstArraySlice = 0;
			dsv_desc.Texture2DArray.ArraySize = resource_desc.DepthOrArraySize;
		}
		device_->CreateDepthStencilView(
			texture_resource.resource.Get(),
			&dsv_desc,
			GetCpuDescriptorHandle(offscreen_dsv_heap_, texture_resource.dsv_descriptor_index));
	}

	const uint64_t texture_handle_value = next_texture_handle_++;
	texture_resources_.emplace(texture_handle_value, std::move(texture_resource));
	texture_handle = RhiTextureHandle{ texture_handle_value };
	return true;
}

bool DirectX12RhiDevice::CreateFramebufferTextureResource(const RhiTextureDesc& desc, bool create_rtv, bool create_dsv, RhiTextureHandle& texture_handle)
{
	return CreateTextureResource(
		desc,
		ToDxgiResourceFormat(desc.format),
		ToDxgiShaderResourceFormat(desc.format),
		ToDxgiRenderTargetFormat(desc.format),
		ToDxgiDepthStencilFormat(desc.format),
		create_rtv,
		create_dsv,
		texture_handle);
}

bool DirectX12RhiDevice::CreateUploadBufferAllocation(size_t committed_size, size_t logical_size, const void* initial_data, UploadBufferAllocation& allocation)
{
	if (device_ == nullptr || committed_size == 0 || logical_size == 0) {
		return false;
	}

	D3D12_HEAP_PROPERTIES heap_properties{};
	heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 1;
	heap_properties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resource_desc{};
	resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resource_desc.Alignment = 0;
	resource_desc.Width = committed_size;
	resource_desc.Height = 1;
	resource_desc.DepthOrArraySize = 1;
	resource_desc.MipLevels = 1;
	resource_desc.Format = DXGI_FORMAT_UNKNOWN;
	resource_desc.SampleDesc.Count = 1;
	resource_desc.SampleDesc.Quality = 0;
	resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	const HRESULT resource_hr = device_->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&resource_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&allocation.resource));
	if (FAILED(resource_hr)) {
		ReportFailure("ID3D12Device::CreateCommittedResource(buffer)", resource_hr);
		return false;
	}

	const HRESULT map_hr = allocation.resource->Map(0, nullptr, reinterpret_cast<void**>(&allocation.mapped_data));
	if (FAILED(map_hr)) {
		ReportFailure("ID3D12Resource::Map(buffer)", map_hr);
		allocation.resource.Reset();
		return false;
	}

	allocation.size = committed_size;
	allocation.logical_size = logical_size;
	allocation.gpu_address = allocation.resource->GetGPUVirtualAddress();
	std::memset(allocation.mapped_data, 0, committed_size);
	if (initial_data != nullptr) {
		std::memcpy(allocation.mapped_data, initial_data, logical_size);
	}
	return true;
}

void DirectX12RhiDevice::ReleaseUploadBufferAllocation(UploadBufferAllocation& allocation)
{
	if (allocation.resource != nullptr && allocation.mapped_data != nullptr) {
		allocation.resource->Unmap(0, nullptr);
	}
	allocation.mapped_data = nullptr;
	allocation.gpu_address = 0;
	allocation.size = 0;
	allocation.logical_size = 0;
	allocation.resource.Reset();
}

void DirectX12RhiDevice::ReleaseTextureResource(TextureResource& texture_resource)
{
	if (texture_resource.srv_descriptor_index != kInvalidDescriptorIndex) {
		FreeDescriptor(cbv_srv_uav_heap_, texture_resource.srv_descriptor_index);
		texture_resource.srv_descriptor_index = kInvalidDescriptorIndex;
	}
	if (texture_resource.rtv_descriptor_index != kInvalidDescriptorIndex) {
		FreeDescriptor(offscreen_rtv_heap_, texture_resource.rtv_descriptor_index);
		texture_resource.rtv_descriptor_index = kInvalidDescriptorIndex;
	}
	if (texture_resource.dsv_descriptor_index != kInvalidDescriptorIndex) {
		FreeDescriptor(offscreen_dsv_heap_, texture_resource.dsv_descriptor_index);
		texture_resource.dsv_descriptor_index = kInvalidDescriptorIndex;
	}
	texture_resource.readback_resource.Reset();
	texture_resource.readback_buffer_size = 0;
	texture_resource.resource.Reset();
	texture_resource.current_state = D3D12_RESOURCE_STATE_COMMON;
}

bool DirectX12RhiDevice::WriteUploadBufferAllocation(UploadBufferAllocation& allocation, size_t offset, size_t size, const void* data)
{
	if (allocation.mapped_data == nullptr || data == nullptr || size == 0 || offset + size > allocation.logical_size) {
		return false;
	}
	std::memcpy(allocation.mapped_data + offset, data, size);
	return true;
}

bool DirectX12RhiDevice::UploadTextureSubresources(TextureResource& texture_resource,
	const std::vector<TextureUploadSubresource>& subresources,
	D3D12_RESOURCE_STATES final_state)
{
	if (device_ == nullptr
		|| command_list_ == nullptr
		|| texture_resource.resource == nullptr
		|| subresources.empty()
		|| subresources.size() != GetTextureSubresourceCount(texture_resource.desc)) {
		return false;
	}

	const D3D12_RESOURCE_DESC resource_desc = texture_resource.resource->GetDesc();
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(subresources.size());
	std::vector<UINT> row_counts(subresources.size());
	std::vector<UINT64> row_sizes(subresources.size());
	UINT64 upload_size = 0;
	device_->GetCopyableFootprints(
		&resource_desc,
		0,
		static_cast<UINT>(subresources.size()),
		0,
		layouts.data(),
		row_counts.data(),
		row_sizes.data(),
		&upload_size);
	if (upload_size == 0) {
		return false;
	}

	D3D12_HEAP_PROPERTIES heap_properties{};
	heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 1;
	heap_properties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC upload_desc{};
	upload_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	upload_desc.Alignment = 0;
	upload_desc.Width = upload_size;
	upload_desc.Height = 1;
	upload_desc.DepthOrArraySize = 1;
	upload_desc.MipLevels = 1;
	upload_desc.Format = DXGI_FORMAT_UNKNOWN;
	upload_desc.SampleDesc.Count = 1;
	upload_desc.SampleDesc.Quality = 0;
	upload_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	upload_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Microsoft::WRL::ComPtr<ID3D12Resource> upload_resource;
	const HRESULT upload_hr = device_->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&upload_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&upload_resource));
	if (FAILED(upload_hr)) {
		ReportFailure("ID3D12Device::CreateCommittedResource(texture_upload)", upload_hr);
		return false;
	}

	std::byte* mapped_data = nullptr;
	const HRESULT map_hr = upload_resource->Map(0, nullptr, reinterpret_cast<void**>(&mapped_data));
	if (FAILED(map_hr) || mapped_data == nullptr) {
		ReportFailure("ID3D12Resource::Map(texture_upload)", map_hr);
		return false;
	}

	for (size_t subresource_index = 0; subresource_index < subresources.size(); ++subresource_index) {
		if (subresources[subresource_index].data == nullptr) {
			upload_resource->Unmap(0, nullptr);
			return false;
		}

		const auto& placed_footprint = layouts[subresource_index];
		const auto* source_bytes = static_cast<const std::byte*>(subresources[subresource_index].data);
		for (UINT row = 0; row < row_counts[subresource_index]; ++row) {
			std::memcpy(
				mapped_data + placed_footprint.Offset + static_cast<size_t>(row) * placed_footprint.Footprint.RowPitch,
				source_bytes + static_cast<size_t>(row) * subresources[subresource_index].row_pitch,
				static_cast<size_t>(row_sizes[subresource_index]));
		}
	}

	upload_resource->Unmap(0, nullptr);

	if (!ResetCommandListForRecording()) {
		return false;
	}
	if (!TransitionTextureResource(texture_resource, D3D12_RESOURCE_STATE_COPY_DEST)) {
		return false;
	}

	for (size_t subresource_index = 0; subresource_index < subresources.size(); ++subresource_index) {
		D3D12_TEXTURE_COPY_LOCATION destination_location{};
		destination_location.pResource = texture_resource.resource.Get();
		destination_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		destination_location.SubresourceIndex = static_cast<UINT>(subresource_index);

		D3D12_TEXTURE_COPY_LOCATION source_location{};
		source_location.pResource = upload_resource.Get();
		source_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		source_location.PlacedFootprint = layouts[subresource_index];

		command_list_->CopyTextureRegion(&destination_location, 0, 0, 0, &source_location, nullptr);
	}

	if (!TransitionTextureResource(texture_resource, final_state)) {
		return false;
	}
	return CloseAndExecuteCommandList();
}

uint32_t DirectX12RhiDevice::AllocateDescriptor(DescriptorHeapState& heap)
{
	if (!heap.free_indices.empty()) {
		const uint32_t descriptor_index = heap.free_indices.back();
		heap.free_indices.pop_back();
		return descriptor_index;
	}
	if (heap.next_index >= heap.capacity) {
		return kInvalidDescriptorIndex;
	}
	return heap.next_index++;
}

void DirectX12RhiDevice::FreeDescriptor(DescriptorHeapState& heap, uint32_t descriptor_index)
{
	if (descriptor_index == kInvalidDescriptorIndex || descriptor_index >= heap.capacity) {
		return;
	}
	heap.free_indices.push_back(descriptor_index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectX12RhiDevice::GetCpuDescriptorHandle(const DescriptorHeapState& heap, uint32_t descriptor_index) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle{};
	if (heap.heap == nullptr) {
		return handle;
	}
	handle = heap.heap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += static_cast<SIZE_T>(descriptor_index) * heap.descriptor_size;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectX12RhiDevice::GetGpuDescriptorHandle(const DescriptorHeapState& heap, uint32_t descriptor_index) const
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle{};
	if (heap.heap == nullptr) {
		return handle;
	}
	handle = heap.heap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += static_cast<UINT64>(descriptor_index) * heap.descriptor_size;
	return handle;
}

bool DirectX12RhiDevice::TransitionTextureResource(TextureResource& texture_resource, D3D12_RESOURCE_STATES new_state)
{
	if (command_list_ == nullptr || texture_resource.resource == nullptr) {
		return false;
	}
	if (texture_resource.current_state == new_state) {
		return true;
	}

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture_resource.resource.Get();
	barrier.Transition.StateBefore = texture_resource.current_state;
	barrier.Transition.StateAfter = new_state;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	command_list_->ResourceBarrier(1, &barrier);
	texture_resource.current_state = new_state;
	return true;
}

void DirectX12RhiDevice::DestroyTextureHandle(uint64_t texture_handle_value)
{
	const auto iter = texture_resources_.find(texture_handle_value);
	if (iter == texture_resources_.end()) {
		return;
	}

	for (auto& [framebuffer_handle, framebuffer_resource] : framebuffer_resources_) {
		(void)framebuffer_handle;
		for (size_t i = 0; i < framebuffer_resource.textures.size(); ++i) {
			if (framebuffer_resource.textures[i].value == texture_handle_value) {
				framebuffer_resource.textures[i] = {};
				if (i < framebuffer_resource.owns_textures.size()) {
					framebuffer_resource.owns_textures[i] = false;
				}
			}
		}
		if (framebuffer_resource.depth_stencil_texture.value == texture_handle_value) {
			framebuffer_resource.depth_stencil_texture = {};
			framebuffer_resource.owns_depth_stencil_texture = false;
		}
	}

	for (auto texture_iter = bound_textures_.begin(); texture_iter != bound_textures_.end();) {
		if (texture_iter->second.value == texture_handle_value) {
			texture_iter = bound_textures_.erase(texture_iter);
		}
		else {
			++texture_iter;
		}
	}

	ReleaseTextureResource(iter->second);
	texture_resources_.erase(iter);
}

void DirectX12RhiDevice::ReleaseFramebufferTextures(FramebufferResource& framebuffer_resource, uint64_t preserved_texture)
{
	std::unordered_set<uint64_t> released_textures;
	for (size_t i = 0; i < framebuffer_resource.textures.size(); ++i) {
		const auto texture = framebuffer_resource.textures[i];
		const bool owns_texture = i < framebuffer_resource.owns_textures.size() && framebuffer_resource.owns_textures[i];
		if (owns_texture && texture.IsValid() && texture.value != preserved_texture && released_textures.insert(texture.value).second) {
			DestroyTextureHandle(texture.value);
		}
	}
	if (framebuffer_resource.owns_depth_stencil_texture
		&& framebuffer_resource.depth_stencil_texture.IsValid()
		&& framebuffer_resource.depth_stencil_texture.value != preserved_texture
		&& released_textures.insert(framebuffer_resource.depth_stencil_texture.value).second) {
		DestroyTextureHandle(framebuffer_resource.depth_stencil_texture.value);
	}
	framebuffer_resource.textures.clear();
	framebuffer_resource.owns_textures.clear();
	framebuffer_resource.depth_stencil_texture = {};
	framebuffer_resource.owns_depth_stencil_texture = false;
}

bool DirectX12RhiDevice::EnsureTextureReadbackResource(TextureResource& texture_resource,
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT& footprint,
	UINT& row_count,
	UINT64& row_size_bytes,
	UINT64& total_bytes)
{
	footprint = {};
	row_count = 0;
	row_size_bytes = 0;
	total_bytes = 0;
	if (device_ == nullptr || texture_resource.resource == nullptr) {
		return false;
	}

	const D3D12_RESOURCE_DESC resource_desc = texture_resource.resource->GetDesc();
	device_->GetCopyableFootprints(&resource_desc, 0, 1, 0, &footprint, &row_count, &row_size_bytes, &total_bytes);
	if (total_bytes == 0) {
		return false;
	}
	if (texture_resource.readback_resource != nullptr && texture_resource.readback_buffer_size >= total_bytes) {
		return true;
	}

	texture_resource.readback_resource.Reset();
	texture_resource.readback_buffer_size = 0;

	D3D12_HEAP_PROPERTIES heap_properties{};
	heap_properties.Type = D3D12_HEAP_TYPE_READBACK;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 1;
	heap_properties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC readback_desc{};
	readback_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	readback_desc.Alignment = 0;
	readback_desc.Width = total_bytes;
	readback_desc.Height = 1;
	readback_desc.DepthOrArraySize = 1;
	readback_desc.MipLevels = 1;
	readback_desc.Format = DXGI_FORMAT_UNKNOWN;
	readback_desc.SampleDesc.Count = 1;
	readback_desc.SampleDesc.Quality = 0;
	readback_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	readback_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	const HRESULT hr = device_->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&readback_desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texture_resource.readback_resource));
	if (FAILED(hr)) {
		ReportFailure("ID3D12Device::CreateCommittedResource(readback)", hr);
		return false;
	}

	texture_resource.readback_buffer_size = total_bytes;
	return true;
}

bool DirectX12RhiDevice::CreateSwapChainForPresentSurface(HWND hwnd, uint32_t width, uint32_t height)
{
	if (factory_ == nullptr || command_queue_ == nullptr || device_ == nullptr || hwnd == nullptr) {
		return false;
	}

	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{};
	swap_chain_desc.Width = width;
	swap_chain_desc.Height = height;
	swap_chain_desc.Format = kDefaultSwapChainFormat;
	swap_chain_desc.Stereo = FALSE;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = kSwapChainBufferCount;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swap_chain_desc.Flags = 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain;
	const HRESULT create_swap_chain_hr = factory_->CreateSwapChainForHwnd(
		command_queue_.Get(),
		hwnd,
		&swap_chain_desc,
		nullptr,
		nullptr,
		&swap_chain);
	if (FAILED(create_swap_chain_hr)) {
		ReportFailure("IDXGIFactory4::CreateSwapChainForHwnd", create_swap_chain_hr);
		return false;
	}

	const HRESULT association_hr = factory_->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	if (FAILED(association_hr)) {
		ReportFailure("IDXGIFactory4::MakeWindowAssociation", association_hr);
	}

	const HRESULT swap_chain_hr = swap_chain.As(&present_surface_state_.swap_chain);
	if (FAILED(swap_chain_hr)) {
		ReportFailure("IDXGISwapChain1::QueryInterface(IDXGISwapChain3)", swap_chain_hr);
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv_heap_desc.NumDescriptors = kSwapChainBufferCount;
	rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtv_heap_desc.NodeMask = 0;

	const HRESULT create_heap_hr = device_->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&present_surface_state_.rtv_heap.heap));
	if (FAILED(create_heap_hr)) {
		ReportFailure("ID3D12Device::CreateDescriptorHeap(RTV)", create_heap_hr);
		present_surface_state_.swap_chain.Reset();
		return false;
	}

	present_surface_state_.rtv_heap.descriptor_size = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	present_surface_state_.rtv_heap.capacity = rtv_heap_desc.NumDescriptors;
	present_surface_state_.rtv_heap.next_index = 0;
	present_surface_state_.rtv_heap.free_indices.clear();
	present_surface_state_.format = swap_chain_desc.Format;
	present_surface_state_.buffer_count = kSwapChainBufferCount;
	present_surface_state_.width = width;
	present_surface_state_.height = height;
	present_surface_state_.current_back_buffer_index = present_surface_state_.swap_chain->GetCurrentBackBufferIndex();
	present_surface_state_.owns_default_render_target = true;
	return true;
}

bool DirectX12RhiDevice::CreateSwapChainBackBuffers()
{
	if (!HasOwnedPresentSurface() || device_ == nullptr) {
		return false;
	}

	ReleaseSwapChainBackBuffers();
	D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{};
	rtv_desc.Format = present_surface_state_.format;
	rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (uint32_t index = 0; index < present_surface_state_.buffer_count; ++index) {
		const HRESULT get_buffer_hr = present_surface_state_.swap_chain->GetBuffer(
			index,
			IID_PPV_ARGS(&present_surface_state_.back_buffers[index]));
		if (FAILED(get_buffer_hr)) {
			ReportFailure("IDXGISwapChain3::GetBuffer", get_buffer_hr);
			ReleaseSwapChainBackBuffers();
			return false;
		}

		device_->CreateRenderTargetView(
			present_surface_state_.back_buffers[index].Get(),
			&rtv_desc,
			GetCpuDescriptorHandle(present_surface_state_.rtv_heap, index));
	}

	present_surface_state_.current_back_buffer_index = present_surface_state_.swap_chain->GetCurrentBackBufferIndex();
	return true;
}

void DirectX12RhiDevice::ReleaseSwapChainBackBuffers()
{
	for (auto& back_buffer : present_surface_state_.back_buffers) {
		back_buffer.Reset();
	}
}

bool DirectX12RhiDevice::ResizeOwnedPresentSurface(uint32_t width, uint32_t height)
{
	if (!HasOwnedPresentSurface() || width == 0 || height == 0) {
		return false;
	}
	if (present_surface_state_.width == width && present_surface_state_.height == height) {
		return true;
	}

	WaitForGpuIdle();
	ReleaseSwapChainBackBuffers();
	const HRESULT hr = present_surface_state_.swap_chain->ResizeBuffers(
		present_surface_state_.buffer_count,
		width,
		height,
		present_surface_state_.format,
		0);
	if (FAILED(hr)) {
		ReportFailure("IDXGISwapChain3::ResizeBuffers", hr);
		return false;
	}

	present_surface_state_.width = width;
	present_surface_state_.height = height;
	return CreateSwapChainBackBuffers();
}

bool DirectX12RhiDevice::ResetCommandListForRecording()
{
	if (command_allocator_ == nullptr || command_list_ == nullptr) {
		return false;
	}

	if (command_list_ready_) {
		const HRESULT close_hr = command_list_->Close();
		if (FAILED(close_hr)) {
			ReportFailure("ID3D12GraphicsCommandList::Close", close_hr);
			return false;
		}
		command_list_ready_ = false;
	}

	const HRESULT reset_allocator_hr = command_allocator_->Reset();
	if (FAILED(reset_allocator_hr)) {
		ReportFailure("ID3D12CommandAllocator::Reset", reset_allocator_hr);
		return false;
	}

	const HRESULT reset_list_hr = command_list_->Reset(command_allocator_.Get(), nullptr);
	if (FAILED(reset_list_hr)) {
		ReportFailure("ID3D12GraphicsCommandList::Reset", reset_list_hr);
		return false;
	}

	command_list_ready_ = true;
	if (cbv_srv_uav_heap_.heap != nullptr) {
		ID3D12DescriptorHeap* descriptor_heaps[] = { cbv_srv_uav_heap_.heap.Get() };
		command_list_->SetDescriptorHeaps(1, descriptor_heaps);
	}
	return true;
}

bool DirectX12RhiDevice::CloseAndExecuteCommandList()
{
	if (!command_list_ready_ || command_list_ == nullptr || command_queue_ == nullptr) {
		return false;
	}

	const HRESULT close_hr = command_list_->Close();
	if (FAILED(close_hr)) {
		ReportFailure("ID3D12GraphicsCommandList::Close", close_hr);
		return false;
	}

	command_list_ready_ = false;
	ID3D12CommandList* command_lists[] = { command_list_.Get() };
	command_queue_->ExecuteCommandLists(1, command_lists);
	WaitForGpuIdle();
	return true;
}

bool DirectX12RhiDevice::ConfigureRenderTargetViewportAndScissor(uint32_t width, uint32_t height)
{
	if (command_list_ == nullptr || width == 0 || height == 0) {
		return false;
	}

	D3D12_VIEWPORT viewport{};
	viewport.TopLeftX = static_cast<float>(viewport_.x);
	viewport.TopLeftY = static_cast<float>(viewport_.y);
	viewport.Width = static_cast<float>(viewport_.width > 0 ? viewport_.width : static_cast<int>(width));
	viewport.Height = static_cast<float>(viewport_.height > 0 ? viewport_.height : static_cast<int>(height));
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	command_list_->RSSetViewports(1, &viewport);

	D3D12_RECT scissor_rect{};
	scissor_rect.left = viewport_.x;
	scissor_rect.top = viewport_.y;
	scissor_rect.right = viewport_.x + static_cast<LONG>(viewport.Width);
	scissor_rect.bottom = viewport_.y + static_cast<LONG>(viewport.Height);
	command_list_->RSSetScissorRects(1, &scissor_rect);
	return true;
}

bool DirectX12RhiDevice::BeginDefaultRenderTargetPass(ID3D12Resource*& back_buffer, D3D12_CPU_DESCRIPTOR_HANDLE& rtv_handle)
{
	back_buffer = nullptr;
	rtv_handle = {};
	if (command_list_ == nullptr || !HasOwnedPresentSurface()) {
		return false;
	}

	back_buffer = GetCurrentBackBufferResource();
	if (back_buffer == nullptr) {
		return false;
	}

	const uint32_t width = viewport_.width > 0 ? static_cast<uint32_t>(viewport_.width) : present_surface_state_.width;
	const uint32_t height = viewport_.height > 0 ? static_cast<uint32_t>(viewport_.height) : present_surface_state_.height;
	if (width == 0 || height == 0) {
		back_buffer = nullptr;
		return false;
	}

	if (!ConfigureRenderTargetViewportAndScissor(width, height)) {
		back_buffer = nullptr;
		return false;
	}

	D3D12_RESOURCE_BARRIER to_render_target{};
	to_render_target.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	to_render_target.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	to_render_target.Transition.pResource = back_buffer;
	to_render_target.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	to_render_target.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	to_render_target.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	command_list_->ResourceBarrier(1, &to_render_target);

	rtv_handle = GetCurrentBackBufferRtv();
	command_list_->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);
	return true;
}

bool DirectX12RhiDevice::BeginFramebufferPass(const FramebufferBindingState& binding, FramebufferPassState& pass_state)
{
	pass_state = {};
	pass_state.is_default = binding.is_default || !binding.framebuffer.IsValid();
	if (pass_state.is_default) {
		if (!BeginDefaultRenderTargetPass(pass_state.default_back_buffer, pass_state.rtv_handles[0])) {
			return false;
		}
		pass_state.rtv_count = 1;
		return true;
	}

	if (command_list_ == nullptr) {
		return false;
	}

	const auto framebuffer_iter = framebuffer_resources_.find(binding.framebuffer.value);
	if (framebuffer_iter == framebuffer_resources_.end()) {
		ReportUnsupportedOnce("DirectX12RhiDevice::BeginFramebufferPass(invalid_framebuffer)");
		return false;
	}

	const FramebufferResource& framebuffer_resource = framebuffer_iter->second;
	if (!ConfigureRenderTargetViewportAndScissor(
		static_cast<uint32_t>(std::max(framebuffer_resource.desc.width, 0)),
		static_cast<uint32_t>(std::max(framebuffer_resource.desc.height, 0)))) {
		return false;
	}

	for (const auto texture_handle : framebuffer_resource.textures) {
		if (!texture_handle.IsValid() || pass_state.rtv_count >= pass_state.rtv_handles.size()) {
			continue;
		}

		const auto texture_iter = texture_resources_.find(texture_handle.value);
		if (texture_iter == texture_resources_.end()
			|| texture_iter->second.resource == nullptr
			|| texture_iter->second.rtv_descriptor_index == kInvalidDescriptorIndex) {
			continue;
		}

		TextureResource& texture_resource = texture_iter->second;
		pass_state.color_previous_states[pass_state.rtv_count] = texture_resource.current_state;
		if (!TransitionTextureResource(texture_resource, D3D12_RESOURCE_STATE_RENDER_TARGET)) {
			return false;
		}
		pass_state.color_textures[pass_state.rtv_count] = &texture_resource;
		pass_state.rtv_handles[pass_state.rtv_count] = GetCpuDescriptorHandle(offscreen_rtv_heap_, texture_resource.rtv_descriptor_index);
		++pass_state.rtv_count;
	}

	if (framebuffer_resource.depth_stencil_texture.IsValid()) {
		const auto depth_iter = texture_resources_.find(framebuffer_resource.depth_stencil_texture.value);
		if (depth_iter != texture_resources_.end()
			&& depth_iter->second.resource != nullptr
			&& depth_iter->second.dsv_descriptor_index != kInvalidDescriptorIndex) {
			TextureResource& depth_texture = depth_iter->second;
			pass_state.depth_previous_state = depth_texture.current_state;
			if (!TransitionTextureResource(depth_texture, D3D12_RESOURCE_STATE_DEPTH_WRITE)) {
				return false;
			}
			pass_state.depth_stencil_texture = &depth_texture;
			pass_state.dsv_handle = GetCpuDescriptorHandle(offscreen_dsv_heap_, depth_texture.dsv_descriptor_index);
			pass_state.has_dsv = true;
		}
	}

	if (pass_state.rtv_count == 0 && !pass_state.has_dsv) {
		ReportUnsupportedOnce("DirectX12RhiDevice::BeginFramebufferPass(empty_framebuffer)");
		return false;
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE* const rtv_handles = pass_state.rtv_count > 0 ? pass_state.rtv_handles.data() : nullptr;
	command_list_->OMSetRenderTargets(pass_state.rtv_count, rtv_handles, FALSE, pass_state.has_dsv ? &pass_state.dsv_handle : nullptr);
	return true;
}

void DirectX12RhiDevice::EndDefaultRenderTargetPass(ID3D12Resource* back_buffer)
{
	if (command_list_ == nullptr || back_buffer == nullptr) {
		return;
	}

	D3D12_RESOURCE_BARRIER to_present{};
	to_present.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	to_present.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	to_present.Transition.pResource = back_buffer;
	to_present.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	to_present.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	to_present.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	command_list_->ResourceBarrier(1, &to_present);
}

void DirectX12RhiDevice::EndFramebufferPass(FramebufferPassState& pass_state)
{
	if (pass_state.is_default) {
		EndDefaultRenderTargetPass(pass_state.default_back_buffer);
		return;
	}

	for (uint32_t i = 0; i < pass_state.rtv_count; ++i) {
		if (pass_state.color_textures[i] != nullptr) {
			TransitionTextureResource(*pass_state.color_textures[i], pass_state.color_previous_states[i]);
		}
	}
	if (pass_state.has_dsv && pass_state.depth_stencil_texture != nullptr) {
		TransitionTextureResource(*pass_state.depth_stencil_texture, pass_state.depth_previous_state);
	}
}

bool DirectX12RhiDevice::HasOwnedPresentSurface() const
{
	return present_surface_state_.owns_default_render_target && present_surface_state_.swap_chain != nullptr;
}

ID3D12Resource* DirectX12RhiDevice::GetCurrentBackBufferResource() const
{
	if (!HasOwnedPresentSurface() || present_surface_state_.current_back_buffer_index >= present_surface_state_.buffer_count) {
		return nullptr;
	}
	return present_surface_state_.back_buffers[present_surface_state_.current_back_buffer_index].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectX12RhiDevice::GetCurrentBackBufferRtv() const
{
	if (!HasOwnedPresentSurface()) {
		return {};
	}
	return GetCpuDescriptorHandle(present_surface_state_.rtv_heap, present_surface_state_.current_back_buffer_index);
}

std::optional<D3D12_GPU_VIRTUAL_ADDRESS> DirectX12RhiDevice::GetBoundUniformBufferGpuAddress(uint32_t binding) const
{
	const auto bound_buffer = GetUniformBufferBinding(binding);
	if (!bound_buffer.has_value()) {
		return std::nullopt;
	}

	const auto buffer_iter = buffer_resources_.find(bound_buffer->value);
	if (buffer_iter == buffer_resources_.end() || buffer_iter->second.desc.usage != RhiBufferUsage::Uniform) {
		return std::nullopt;
	}
	return buffer_iter->second.allocation.gpu_address;
}

std::optional<uint32_t> DirectX12RhiDevice::GetBoundTextureSrvDescriptorIndex(uint32_t unit) const
{
	const auto bound_texture = bound_textures_.find(unit);
	if (bound_texture == bound_textures_.end()) {
		return std::nullopt;
	}

	const auto texture_iter = texture_resources_.find(bound_texture->second.value);
	if (texture_iter == texture_resources_.end() || texture_iter->second.srv_descriptor_index == kInvalidDescriptorIndex) {
		return std::nullopt;
	}
	return texture_iter->second.srv_descriptor_index;
}

Dx12BuiltinPassShader DirectX12RhiDevice::ResolveBuiltinPassShader(RhiVertexLayout vertex_layout) const
{
	if (active_builtin_shader_ == Dx12BuiltinPassShader::DepthOnly || active_builtin_shader_ == Dx12BuiltinPassShader::Skybox) {
		return active_builtin_shader_;
	}
	if (VertexLayoutSupportsTexcoords(vertex_layout) && GetBoundTextureSrvDescriptorIndex(kDx12BuiltinTexture2DBinding).has_value()) {
		return Dx12BuiltinPassShader::MeshTextured2D;
	}
	return Dx12BuiltinPassShader::MeshColor;
}

bool DirectX12RhiDevice::ResolveDrawFramebufferFormats(const FramebufferBindingState& binding, DXGI_FORMAT& render_target_format, DXGI_FORMAT& depth_stencil_format) const
{
	render_target_format = DXGI_FORMAT_UNKNOWN;
	depth_stencil_format = DXGI_FORMAT_UNKNOWN;

	if (binding.is_default || !binding.framebuffer.IsValid()) {
		if (!HasOwnedPresentSurface()) {
			ReportUnsupportedOnce("DirectX12RhiDevice::DrawMesh(default_render_target_unavailable)");
			return false;
		}
		render_target_format = present_surface_state_.format;
		return true;
	}

	const auto framebuffer_iter = framebuffer_resources_.find(binding.framebuffer.value);
	if (framebuffer_iter == framebuffer_resources_.end()) {
		ReportUnsupportedOnce("DirectX12RhiDevice::DrawMesh(invalid_framebuffer)");
		return false;
	}

	for (const auto texture_handle : framebuffer_iter->second.textures) {
		if (!texture_handle.IsValid()) {
			continue;
		}

		const auto texture_iter = texture_resources_.find(texture_handle.value);
		if (texture_iter == texture_resources_.end()) {
			continue;
		}

		const TextureResource& texture_resource = texture_iter->second;
		if (texture_resource.resource != nullptr
			&& texture_resource.rtv_descriptor_index != kInvalidDescriptorIndex
			&& texture_resource.render_target_format != DXGI_FORMAT_UNKNOWN) {
			render_target_format = texture_resource.render_target_format;
			break;
		}
	}

	const RhiTextureHandle depth_texture_handle = framebuffer_iter->second.depth_stencil_texture;
	if (depth_texture_handle.IsValid()) {
		const auto depth_iter = texture_resources_.find(depth_texture_handle.value);
		if (depth_iter != texture_resources_.end()) {
			const TextureResource& depth_texture = depth_iter->second;
			if (depth_texture.resource != nullptr
				&& depth_texture.dsv_descriptor_index != kInvalidDescriptorIndex
				&& depth_texture.depth_stencil_format != DXGI_FORMAT_UNKNOWN) {
				depth_stencil_format = depth_texture.depth_stencil_format;
			}
		}
	}

	if (active_builtin_shader_ == Dx12BuiltinPassShader::DepthOnly && depth_stencil_format != DXGI_FORMAT_UNKNOWN) {
		return true;
	}

	if (render_target_format == DXGI_FORMAT_UNKNOWN) {
		ReportUnsupportedOnce("DirectX12RhiDevice::DrawMesh(color_render_target_unavailable)");
		return false;
	}

	return true;
}

ID3D12PipelineState* DirectX12RhiDevice::GetBuiltinMeshPipelineState(const FramebufferBindingState& binding, RhiVertexLayout vertex_layout, RhiPrimitiveTopology topology)
{
	if (builtin_mesh_pipeline_ == nullptr) {
		return nullptr;
	}

	const Dx12BuiltinPassShader resolved_shader = ResolveBuiltinPassShader(vertex_layout);

	DXGI_FORMAT render_target_format = DXGI_FORMAT_UNKNOWN;
	DXGI_FORMAT depth_stencil_format = DXGI_FORMAT_UNKNOWN;
	if (!ResolveDrawFramebufferFormats(binding, render_target_format, depth_stencil_format)) {
		return nullptr;
	}
	if (depth_test_enabled_ && depth_stencil_format == DXGI_FORMAT_UNKNOWN) {
		if (binding.is_default || !binding.framebuffer.IsValid()) {
			ReportUnsupportedOnce("DirectX12RhiDevice::DrawMesh(default_depth_buffer_unavailable)");
		}
		else {
			ReportUnsupportedOnce("DirectX12RhiDevice::DrawMesh(depth_buffer_unavailable)");
		}
	}

	Dx12BuiltinMeshPipelineDesc pipeline_desc{};
	pipeline_desc.render_target_format = render_target_format;
	pipeline_desc.depth_stencil_format = depth_stencil_format;
	pipeline_desc.vertex_layout = vertex_layout;
	pipeline_desc.topology_type = ToD3DPrimitiveTopologyType(topology);
	pipeline_desc.shader = resolved_shader;
	pipeline_desc.fill_mode = polygon_mode_ == RhiPolygonMode::Line ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
	pipeline_desc.cull_mode = cull_face_enabled_
		? (cull_face_ == RhiCullFace::Front ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_BACK)
		: D3D12_CULL_MODE_NONE;
	pipeline_desc.enable_alpha_blend = blend_enabled_ || blend_alpha_enabled_;
	pipeline_desc.enable_depth_test = depth_test_enabled_ && depth_stencil_format != DXGI_FORMAT_UNKNOWN;
	pipeline_desc.depth_func = ToD3DComparisonFunc(depth_func_);

	std::string error_message;
	ID3D12PipelineState* const pipeline_state = builtin_mesh_pipeline_->GetOrCreatePipeline(pipeline_desc, &error_message);
	if (pipeline_state == nullptr && !error_message.empty()) {
		std::printf("DirectX12RhiDevice: %s.\n", error_message.c_str());
	}
	return pipeline_state;
}

bool DirectX12RhiDevice::EnsureUniformBufferDescriptor(BufferResource& buffer)
{
	if (device_ == nullptr || buffer.desc.usage != RhiBufferUsage::Uniform || buffer.allocation.resource == nullptr) {
		return false;
	}

	if (buffer.descriptor_index == kInvalidDescriptorIndex) {
		buffer.descriptor_index = AllocateDescriptor(cbv_srv_uav_heap_);
		if (buffer.descriptor_index == kInvalidDescriptorIndex) {
			ReportUnsupportedOnce("DirectX12RhiDevice::BindUniformBufferBase(descriptor_heap_exhausted)");
			return false;
		}
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{};
	cbv_desc.BufferLocation = buffer.allocation.gpu_address;
	cbv_desc.SizeInBytes = static_cast<UINT>(buffer.allocation.size);
	device_->CreateConstantBufferView(&cbv_desc, GetCpuDescriptorHandle(cbv_srv_uav_heap_, buffer.descriptor_index));
	return true;
}

void DirectX12RhiDevice::RefreshCapabilities()
{
	capabilities_.backend = RhiBackendType::DirectX12;
	capabilities_.supports_debug_groups = false;
	capabilities_.supports_texture_2d_array = true;
	capabilities_.supports_texture_cubemap = true;
	capabilities_.supports_multiple_color_attachments = true;
	capabilities_.supports_framebuffer_readback = true;
	capabilities_.max_color_attachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
	capabilities_.max_texture_array_layers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;

	D3D12_FEATURE_DATA_D3D12_OPTIONS options{};
	if (FAILED(device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)))) {
		return;
	}

	if (options.ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1) {
		capabilities_.max_color_attachments = std::min<uint32_t>(
			capabilities_.max_color_attachments,
			static_cast<uint32_t>(D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT));
	}
}

void DirectX12RhiDevice::BindFramebufferState(RhiFramebufferBindTarget target, const FramebufferBindingState& binding)
{
	switch (target) {
	case RhiFramebufferBindTarget::Framebuffer:
		draw_framebuffer_binding_ = binding;
		read_framebuffer_binding_ = binding;
		break;
	case RhiFramebufferBindTarget::Draw:
		draw_framebuffer_binding_ = binding;
		break;
	case RhiFramebufferBindTarget::Read:
		read_framebuffer_binding_ = binding;
		break;
	}
}

void DirectX12RhiDevice::ResetFramebufferBindingStates()
{
	draw_framebuffer_binding_ = {};
	read_framebuffer_binding_ = {};
}

const DirectX12RhiDevice::FramebufferBindingState& DirectX12RhiDevice::GetFramebufferBindingState(RhiFramebufferBindTarget target) const
{
	switch (target) {
	case RhiFramebufferBindTarget::Read:
		return read_framebuffer_binding_;
	case RhiFramebufferBindTarget::Framebuffer:
	case RhiFramebufferBindTarget::Draw:
		return draw_framebuffer_binding_;
	}

	return draw_framebuffer_binding_;
}

void DirectX12RhiDevice::ReportUnsupportedOnce(std::string_view operation) const
{
	const std::string key(operation);
	if (!unsupported_operations_.insert(key).second) {
		return;
	}
	std::printf("%s is not implemented in the DirectX12 MVP backend yet.\n", key.c_str());
}

void DirectX12RhiDevice::ReportFailure(const char* operation, HRESULT hr) const
{
	std::printf("DirectX12RhiDevice: %s failed (HRESULT=0x%08lx).\n", operation, static_cast<unsigned long>(hr));
}

void DirectX12RhiDevice::WaitForGpuIdle()
{
	if (!initialized_ || command_queue_ == nullptr || fence_ == nullptr || fence_event_ == nullptr) {
		return;
	}

	const uint64_t fence_value = SignalFence();
	if (fence_value == 0) {
		return;
	}

	if (fence_->GetCompletedValue() >= fence_value) {
		return;
	}

	const HRESULT hr = fence_->SetEventOnCompletion(fence_value, fence_event_);
	if (FAILED(hr)) {
		ReportFailure("ID3D12Fence::SetEventOnCompletion", hr);
		return;
	}
	WaitForSingleObject(fence_event_, INFINITE);
}

void DirectX12RhiDevice::ResetDeviceState()
{
	WaitForGpuIdle();
	ReleaseSwapChainBackBuffers();
	framebuffer_resources_.clear();
	for (auto& [handle, texture_resource] : texture_resources_) {
		(void)handle;
		ReleaseTextureResource(texture_resource);
	}
	texture_resources_.clear();
	for (auto& [handle, buffer_resource] : buffer_resources_) {
		(void)handle;
		ReleaseUploadBufferAllocation(buffer_resource.allocation);
	}
	buffer_resources_.clear();
	for (auto& [handle, mesh_resource] : mesh_resources_) {
		(void)handle;
		ReleaseUploadBufferAllocation(mesh_resource.vertex_buffer);
		ReleaseUploadBufferAllocation(mesh_resource.index_buffer);
	}
	mesh_resources_.clear();
	bound_buffers_.clear();
	bound_textures_.clear();
	uniform_buffer_bindings_.clear();
	ResetFramebufferBindingStates();
	active_builtin_shader_ = Dx12BuiltinPassShader::MeshColor;
	initialized_ = false;
	command_list_ready_ = false;
	next_buffer_handle_ = 1;
	next_texture_handle_ = 1;
	next_framebuffer_handle_ = 1;
	next_mesh_handle_ = 1;
	next_fence_value_ = 0;
	present_surface_state_ = {};
	fence_.Reset();
	cbv_srv_uav_heap_ = {};
	offscreen_rtv_heap_ = {};
	offscreen_dsv_heap_ = {};
	builtin_mesh_pipeline_.reset();
	command_list_.Reset();
	command_allocator_.Reset();
	command_queue_.Reset();
	device_.Reset();
	adapter_.Reset();
	factory_.Reset();
	if (fence_event_ != nullptr) {
		CloseHandle(fence_event_);
		fence_event_ = nullptr;
	}
}

uint64_t DirectX12RhiDevice::SignalFence()
{
	if (command_queue_ == nullptr || fence_ == nullptr) {
		return 0;
	}

	const uint64_t fence_value = ++next_fence_value_;
	const HRESULT hr = command_queue_->Signal(fence_.Get(), fence_value);
	if (FAILED(hr)) {
		ReportFailure("ID3D12CommandQueue::Signal", hr);
		return 0;
	}
	return fence_value;
}

D3D_PRIMITIVE_TOPOLOGY DirectX12RhiDevice::ToD3DPrimitiveTopology(RhiPrimitiveTopology topology)
{
	switch (topology) {
	case RhiPrimitiveTopology::Points:
		return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	case RhiPrimitiveTopology::Lines:
		return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	case RhiPrimitiveTopology::LineStrip:
		return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case RhiPrimitiveTopology::Triangles:
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}
	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

Microsoft::WRL::ComPtr<IDXGIAdapter1> DirectX12RhiDevice::FindHardwareAdapter() const
{
	if (factory_ == nullptr) {
		return nullptr;
	}

	Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(factory_.As(&factory6))) {
		for (UINT index = 0;; ++index) {
			Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
			const HRESULT hr = factory6->EnumAdapterByGpuPreference(
				index,
				ToDxgiGpuPreference(init_config_.adapter_preference),
				IID_PPV_ARGS(&adapter));
			if (hr == DXGI_ERROR_NOT_FOUND) {
				break;
			}
			if (FAILED(hr) || !IsHardwareAdapter(adapter.Get())) {
				continue;
			}

			Microsoft::WRL::ComPtr<ID3D12Device> test_device;
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&test_device)))) {
				return adapter;
			}
		}
		return nullptr;
	}

	for (UINT index = 0;; ++index) {
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		const HRESULT hr = factory_->EnumAdapters1(index, &adapter);
		if (hr == DXGI_ERROR_NOT_FOUND) {
			break;
		}
		if (FAILED(hr) || !IsHardwareAdapter(adapter.Get())) {
			continue;
		}

		Microsoft::WRL::ComPtr<ID3D12Device> test_device;
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&test_device)))) {
			return adapter;
		}
	}

	return nullptr;
}

Microsoft::WRL::ComPtr<IDXGIAdapter1> DirectX12RhiDevice::FindWarpAdapter() const
{
	if (factory_ == nullptr) {
		return nullptr;
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter> warp_adapter;
	const HRESULT hr = factory_->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter));
	if (FAILED(hr)) {
		ReportFailure("IDXGIFactory4::EnumWarpAdapter", hr);
		return nullptr;
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter1> warp_adapter_1;
	if (FAILED(warp_adapter.As(&warp_adapter_1))) {
		return nullptr;
	}
	return warp_adapter_1;
}

std::shared_ptr<DirectX12RhiDevice> AsDirectX12RhiDevice(const std::shared_ptr<IRhiDevice>& device)
{
	return std::dynamic_pointer_cast<DirectX12RhiDevice>(device);
}

std::shared_ptr<IRhiDevice> CreateDirectX12RhiDevice()
{
	return CreateDirectX12RhiDevice({});
}

std::shared_ptr<IRhiDevice> CreateDirectX12RhiDevice(const RhiDeviceInitConfig& config)
{
	if (!IsRhiBackendSupported(RhiBackendType::DirectX12)) {
		return nullptr;
	}

	RhiDeviceInitConfig dx12_config = config;
	dx12_config.backend = RhiBackendType::DirectX12;

	auto device = std::make_shared<DirectX12RhiDevice>(dx12_config);
	device->Initialize(dx12_config);
	if (!device->IsInitialized()) {
		return nullptr;
	}
	return device;
}

} // namespace GComponent
