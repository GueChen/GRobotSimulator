#ifndef GSIM_RENDER_RHI_DIRECTX12_DEVICE_H
#define GSIM_RENDER_RHI_DIRECTX12_DEVICE_H

#include "render/rhi/dx12/dx12_common.h"
#include "render/rhi/dx12/dx12_builtin_mesh_pipeline.h"
#include "render/rhi/rhi_device.h"

#include <cstddef>
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace GComponent {

class Dx12BuiltinMeshPipelineLibrary;

class DirectX12RhiDevice final : public IRhiDevice {
public:
	DirectX12RhiDevice();
	explicit DirectX12RhiDevice(const RhiDeviceInitConfig& init_config);
	~DirectX12RhiDevice() override;

	[[nodiscard]] RhiBackendType GetBackendType() const override;
	void Initialize() override;
	void Initialize(const RhiDeviceInitConfig& config) override;
	void InitializeForSurface(const RhiDeviceInitConfig& config, const RhiPresentSurfaceDesc& surface) override;
	[[nodiscard]] const RhiDeviceInitConfig& GetInitConfig() const override;
	[[nodiscard]] RhiDeviceCapabilities GetCapabilities() const override;

	void Enable(RhiCapability capability) override;
	void Disable(RhiCapability capability) override;
	void SetDepthFunc(RhiDepthFunc func) override;
	void SetCullFace(RhiCullFace face) override;
	void SetPolygonMode(RhiPolygonMode mode) override;
	void SetBlendAlpha() override;
	void SetLineWidth(float width) override;
	void SetViewport(const RhiViewport& viewport) override;
	void SetClearColor(const RhiClearColor& color) override;
	void Clear(RhiClearFlags flags) override;
	void PushDebugGroup(std::string_view name) override;
	void PopDebugGroup() override;
	void BindShader(const RhiShaderDesc* shader_desc) override;
	void BindTextureUnit(uint32_t unit, RhiTextureHandle texture) override;
	void BindDefaultFramebuffer() override;
	void BindDefaultFramebuffer(RhiFramebufferBindTarget target) override;
	void BindFramebuffer(RhiFramebufferBindTarget target, RhiFramebufferHandle framebuffer) override;
	uint32_t GetDefaultFramebuffer() const override;
	void ResizePresentSurface(uint32_t width, uint32_t height) override;
	void Present() override;

	RhiBufferHandle CreateBuffer(const RhiBufferDesc& desc, const void* initial_data = nullptr) override;
	void BindBuffer(RhiBufferUsage usage, RhiBufferHandle buffer) override;
	void BindUniformBufferBase(uint32_t binding, RhiBufferHandle buffer) override;
	void UpdateBuffer(RhiBufferHandle buffer, size_t offset, size_t size, const void* data) override;
	void DestroyBuffer(RhiBufferHandle buffer) override;

	RhiTextureHandle CreateTexture(const RhiTextureDesc& desc, const void* initial_data = nullptr) override;
	void DestroyTexture(RhiTextureHandle texture) override;
	RhiTextureHandle LoadTexture2D(std::string_view path, bool repeat = true) override;
	RhiTextureHandle LoadCubemap(const std::vector<std::string_view>& paths) override;

	RhiFramebufferHandle CreatePickingFramebuffer(int width, int height) override;
	RhiFramebufferHandle CreateFramebuffer(const RhiFramebufferCreateDesc& desc) override;
	void DestroyFramebuffer(RhiFramebufferHandle framebuffer) override;
	RhiTextureHandle GetFramebufferTexture(RhiFramebufferHandle framebuffer) const override;
	RhiTextureHandle GetFramebufferColorTexture(RhiFramebufferHandle framebuffer, uint32_t index) const override;
	RhiTextureHandle TakeFramebufferTexture(RhiFramebufferHandle framebuffer) override;
	RhiTextureHandle ReallocateFramebufferTexture(RhiFramebufferHandle framebuffer, const RhiFramebufferCreateDesc& desc) override;
	void ResizeFramebufferRenderbuffer(RhiFramebufferHandle framebuffer, int width, int height) override;
	void ReadFramebufferColorPixel(RhiFramebufferHandle framebuffer, uint32_t x, uint32_t y, float* rgb) override;

	RhiMeshHandle CreateMesh(const RhiMeshDesc& desc) override;
	void UpdateMeshVertexData(RhiMeshHandle mesh, size_t offset, size_t size, const void* data) override;
	void DestroyMesh(RhiMeshHandle mesh) override;
	void DrawMesh(RhiMeshHandle mesh, RhiPrimitiveTopology topology, uint32_t index_count) override;

	[[nodiscard]] bool IsInitialized() const { return initialized_; }
	[[nodiscard]] IDXGIFactory4* GetDxgiFactoryRaw() const { return factory_.Get(); }
	[[nodiscard]] IDXGIAdapter4* GetAdapterRaw() const { return adapter_.Get(); }
	[[nodiscard]] ID3D12Device* GetDeviceRaw() const { return device_.Get(); }
	[[nodiscard]] ID3D12CommandQueue* GetCommandQueueRaw() const { return command_queue_.Get(); }
	[[nodiscard]] ID3D12Fence* GetFenceRaw() const { return fence_.Get(); }
	[[nodiscard]] ID3D12CommandAllocator* GetGraphicsCommandAllocatorRaw() const { return command_allocator_.Get(); }
	[[nodiscard]] ID3D12GraphicsCommandList* GetGraphicsCommandListRaw() const { return command_list_.Get(); }
	[[nodiscard]] ID3D12DescriptorHeap* GetCbvSrvUavHeapRaw() const { return cbv_srv_uav_heap_.heap.Get(); }
	[[nodiscard]] size_t GetMeshResourceCount() const { return mesh_resources_.size(); }
	[[nodiscard]] std::optional<RhiBufferHandle> GetUniformBufferBinding(uint32_t binding) const;

	struct TextureUploadSubresource {
		const void* data = nullptr;
		size_t row_pitch = 0;
		size_t slice_pitch = 0;
	};

private:
	struct UploadBufferAllocation {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		std::byte* mapped_data = nullptr;
		size_t size = 0;
		size_t logical_size = 0;
		D3D12_GPU_VIRTUAL_ADDRESS gpu_address = 0;
	};

	struct BufferResource {
		UploadBufferAllocation allocation;
		RhiBufferDesc desc{};
		uint32_t descriptor_index = UINT32_MAX;
	};

	struct TextureResource {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		Microsoft::WRL::ComPtr<ID3D12Resource> readback_resource;
		RhiTextureDesc desc{};
		DXGI_FORMAT resource_format = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT shader_resource_format = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT render_target_format = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT depth_stencil_format = DXGI_FORMAT_UNKNOWN;
		D3D12_RESOURCE_STATES current_state = D3D12_RESOURCE_STATE_COMMON;
		uint32_t srv_descriptor_index = UINT32_MAX;
		uint32_t rtv_descriptor_index = UINT32_MAX;
		uint32_t dsv_descriptor_index = UINT32_MAX;
		uint64_t readback_buffer_size = 0;
		bool is_depth_stencil = false;
	};

	struct FramebufferResource {
		std::vector<RhiTextureHandle> textures;
		std::vector<bool> owns_textures;
		RhiTextureHandle depth_stencil_texture{};
		bool owns_depth_stencil_texture = false;
		RhiFramebufferCreateDesc desc{};
	};

	struct MeshResource {
		UploadBufferAllocation vertex_buffer;
		UploadBufferAllocation index_buffer;
		size_t vertex_stride = 0;
		size_t vertex_count = 0;
		size_t index_count = 0;
		RhiVertexLayout vertex_layout = RhiVertexLayout::Position3;
		D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view{};
		D3D12_INDEX_BUFFER_VIEW index_buffer_view{};
		bool has_index_buffer = false;
	};

	struct DescriptorHeapState {
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
		uint32_t descriptor_size = 0;
		uint32_t capacity = 0;
		uint32_t next_index = 0;
		std::vector<uint32_t> free_indices;
	};

	struct FramebufferBindingState {
		RhiFramebufferHandle framebuffer{};
		bool is_default = true;
	};

	struct FramebufferPassState {
		bool is_default = true;
		ID3D12Resource* default_back_buffer = nullptr;
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> rtv_handles{};
		std::array<TextureResource*, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> color_textures{};
		std::array<D3D12_RESOURCE_STATES, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> color_previous_states{};
		TextureResource* depth_stencil_texture = nullptr;
		D3D12_RESOURCE_STATES depth_previous_state = D3D12_RESOURCE_STATE_COMMON;
		D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle{};
		uint32_t rtv_count = 0;
		bool has_dsv = false;
	};

	struct PresentSurfaceState {
		Microsoft::WRL::ComPtr<IDXGISwapChain3> swap_chain;
		DescriptorHeapState rtv_heap{};
		std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> back_buffers{};
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		uint32_t buffer_count = 0;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t current_back_buffer_index = 0;
		bool owns_default_render_target = false;
	};

	static DXGI_GPU_PREFERENCE ToDxgiGpuPreference(RhiAdapterPreference preference);
	static D3D_PRIMITIVE_TOPOLOGY ToD3DPrimitiveTopology(RhiPrimitiveTopology topology);
	static D3D12_PRIMITIVE_TOPOLOGY_TYPE ToD3DPrimitiveTopologyType(RhiPrimitiveTopology topology);
	static D3D12_COMPARISON_FUNC ToD3DComparisonFunc(RhiDepthFunc func);
	static size_t GetCommittedBufferSize(const RhiBufferDesc& desc);
	void EnableDebugLayerIfRequested();
	bool CreateFactory();
	bool CreateAdapter();
	bool CreateDevice();
	bool CreateCommandQueue();
	bool CreateCommandObjects();
	bool CreateDescriptorHeaps();
	bool CreateFence();
	bool CreatePresentSurfaceResources();
	bool CreateBuiltinMeshPipelineResources();
	bool CreateTextureResource(const RhiTextureDesc& desc,
		DXGI_FORMAT resource_format,
		DXGI_FORMAT shader_resource_format,
		DXGI_FORMAT render_target_format,
		DXGI_FORMAT depth_stencil_format,
		bool create_rtv,
		bool create_dsv,
		RhiTextureHandle& texture_handle);
	bool CreateFramebufferTextureResource(const RhiTextureDesc& desc, bool create_rtv, bool create_dsv, RhiTextureHandle& texture_handle);
	bool CreateUploadBufferAllocation(size_t committed_size, size_t logical_size, const void* initial_data, UploadBufferAllocation& allocation);
	void ReleaseUploadBufferAllocation(UploadBufferAllocation& allocation);
	void ReleaseTextureResource(TextureResource& texture_resource);
	bool WriteUploadBufferAllocation(UploadBufferAllocation& allocation, size_t offset, size_t size, const void* data);
	bool UploadTextureSubresources(TextureResource& texture_resource,
		const std::vector<TextureUploadSubresource>& subresources,
		D3D12_RESOURCE_STATES final_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	uint32_t AllocateDescriptor(DescriptorHeapState& heap);
	void FreeDescriptor(DescriptorHeapState& heap, uint32_t descriptor_index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(const DescriptorHeapState& heap, uint32_t descriptor_index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(const DescriptorHeapState& heap, uint32_t descriptor_index) const;
	bool TransitionTextureResource(TextureResource& texture_resource, D3D12_RESOURCE_STATES new_state);
	void DestroyTextureHandle(uint64_t texture_handle_value);
	void ReleaseFramebufferTextures(FramebufferResource& framebuffer_resource, uint64_t preserved_texture = 0);
	bool EnsureTextureReadbackResource(TextureResource& texture_resource, D3D12_PLACED_SUBRESOURCE_FOOTPRINT& footprint, UINT& row_count, UINT64& row_size_bytes, UINT64& total_bytes);
	bool CreateSwapChainForPresentSurface(HWND hwnd, uint32_t width, uint32_t height);
	bool CreateSwapChainBackBuffers();
	void ReleaseSwapChainBackBuffers();
	bool ResizeOwnedPresentSurface(uint32_t width, uint32_t height);
	bool ResetCommandListForRecording();
	bool CloseAndExecuteCommandList();
	bool ConfigureRenderTargetViewportAndScissor(uint32_t width, uint32_t height);
	bool BeginDefaultRenderTargetPass(ID3D12Resource*& back_buffer, D3D12_CPU_DESCRIPTOR_HANDLE& rtv_handle);
	void EndDefaultRenderTargetPass(ID3D12Resource* back_buffer);
	bool BeginFramebufferPass(const FramebufferBindingState& binding, FramebufferPassState& pass_state);
	void EndFramebufferPass(FramebufferPassState& pass_state);
	[[nodiscard]] bool HasOwnedPresentSurface() const;
	[[nodiscard]] ID3D12Resource* GetCurrentBackBufferResource() const;
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRtv() const;
	[[nodiscard]] std::optional<D3D12_GPU_VIRTUAL_ADDRESS> GetBoundUniformBufferGpuAddress(uint32_t binding) const;
	[[nodiscard]] std::optional<uint32_t> GetBoundTextureSrvDescriptorIndex(uint32_t unit) const;
	[[nodiscard]] Dx12BuiltinPassShader ResolveBuiltinPassShader(RhiVertexLayout vertex_layout) const;
	[[nodiscard]] bool ResolveDrawFramebufferFormats(const FramebufferBindingState& binding, DXGI_FORMAT& render_target_format, DXGI_FORMAT& depth_stencil_format) const;
	[[nodiscard]] ID3D12PipelineState* GetBuiltinMeshPipelineState(const FramebufferBindingState& binding, RhiVertexLayout vertex_layout, RhiPrimitiveTopology topology);
	bool EnsureUniformBufferDescriptor(BufferResource& buffer);
	void RefreshCapabilities();
	void BindFramebufferState(RhiFramebufferBindTarget target, const FramebufferBindingState& binding);
	void ResetFramebufferBindingStates();
	[[nodiscard]] const FramebufferBindingState& GetFramebufferBindingState(RhiFramebufferBindTarget target) const;
	void ReportUnsupportedOnce(std::string_view operation) const;
	void ReportFailure(const char* operation, HRESULT hr) const;
	void WaitForGpuIdle();
	void ResetDeviceState();
	uint64_t SignalFence();
	Microsoft::WRL::ComPtr<IDXGIAdapter1> FindHardwareAdapter() const;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> FindWarpAdapter() const;

private:
	RhiDeviceInitConfig init_config_{};
	RhiDeviceCapabilities capabilities_{};
	bool initialized_ = false;
	bool command_list_ready_ = false;
	RhiViewport viewport_{};
	RhiClearColor clear_color_{};
	RhiDepthFunc depth_func_ = RhiDepthFunc::Less;
	RhiCullFace cull_face_ = RhiCullFace::Back;
	RhiPolygonMode polygon_mode_ = RhiPolygonMode::Fill;
	float line_width_ = 1.0f;
	bool blend_alpha_enabled_ = false;
	bool depth_test_enabled_ = false;
	bool blend_enabled_ = false;
	bool multisample_enabled_ = false;
	bool cull_face_enabled_ = false;
	Dx12BuiltinPassShader active_builtin_shader_ = Dx12BuiltinPassShader::MeshColor;
	mutable std::unordered_set<std::string> unsupported_operations_;
	uint64_t next_buffer_handle_ = 1;
	uint64_t next_texture_handle_ = 1;
	uint64_t next_framebuffer_handle_ = 1;
	uint64_t next_mesh_handle_ = 1;
	uint64_t next_fence_value_ = 0;
	std::unordered_map<uint64_t, BufferResource> buffer_resources_;
	std::unordered_map<uint64_t, TextureResource> texture_resources_;
	std::unordered_map<uint64_t, FramebufferResource> framebuffer_resources_;
	std::unordered_map<uint64_t, MeshResource> mesh_resources_;
	std::unordered_map<RhiBufferUsage, RhiBufferHandle> bound_buffers_;
	std::unordered_map<uint32_t, RhiTextureHandle> bound_textures_;
	std::unordered_map<uint32_t, RhiBufferHandle> uniform_buffer_bindings_;
	FramebufferBindingState draw_framebuffer_binding_{};
	FramebufferBindingState read_framebuffer_binding_{};
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory_;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list_;
	DescriptorHeapState cbv_srv_uav_heap_{};
	DescriptorHeapState offscreen_rtv_heap_{};
	DescriptorHeapState offscreen_dsv_heap_{};
	PresentSurfaceState present_surface_state_{};
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	HANDLE fence_event_ = nullptr;
	std::unique_ptr<Dx12BuiltinMeshPipelineLibrary> builtin_mesh_pipeline_;
};

std::shared_ptr<DirectX12RhiDevice> AsDirectX12RhiDevice(const std::shared_ptr<IRhiDevice>& device);

} // namespace GComponent

#endif // GSIM_RENDER_RHI_DIRECTX12_DEVICE_H
