#include "render/rhi/ibl_resource_setup.h"

#ifdef GSIM_RENDER_BACKEND_DX12
#include "render/rhi/dx12/directx12_rhi_device.h"
#endif
#include "render/rhi/opengl/opengl_ibl_precompute.h"

#include <stb_image.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

namespace GComponent {

namespace {

#ifdef GSIM_RENDER_BACKEND_DX12
struct FloatImage {
	int width = 0;
	int height = 0;
	std::vector<float> pixels;
};

struct Float3 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

Float3 Normalize(Float3 value)
{
	const float length = std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
	if (length <= std::numeric_limits<float>::epsilon()) {
		return { 0.0f, 0.0f, 1.0f };
	}
	return { value.x / length, value.y / length, value.z / length };
}

FloatImage LoadFloatImage(std::string_view path)
{
	FloatImage image;
	if (path.empty()) {
		return image;
	}

	const std::string owned_path(path);
	int channel_count = 0;
	if (stbi_is_hdr(owned_path.c_str()) != 0) {
		float* data = stbi_loadf(owned_path.c_str(), &image.width, &image.height, &channel_count, STBI_rgb_alpha);
		if (data == nullptr || image.width <= 0 || image.height <= 0) {
			stbi_image_free(data);
			return {};
		}

		const size_t pixel_count = static_cast<size_t>(image.width) * static_cast<size_t>(image.height) * 4u;
		image.pixels.assign(data, data + pixel_count);
		stbi_image_free(data);
		return image;
	}

	stbi_uc* data = stbi_load(owned_path.c_str(), &image.width, &image.height, &channel_count, STBI_rgb_alpha);
	if (data == nullptr || image.width <= 0 || image.height <= 0) {
		stbi_image_free(data);
		return {};
	}

	const size_t pixel_count = static_cast<size_t>(image.width) * static_cast<size_t>(image.height) * 4u;
	image.pixels.resize(pixel_count);
	for (size_t i = 0; i < pixel_count; ++i) {
		image.pixels[i] = static_cast<float>(data[i]) / 255.0f;
	}
	stbi_image_free(data);
	return image;
}

Float3 GetCubemapDirection(uint32_t face_index, float u, float v)
{
	switch (face_index) {
	case 0: return Normalize({ 1.0f, v, -u });
	case 1: return Normalize({ -1.0f, v, u });
	case 2: return Normalize({ u, 1.0f, -v });
	case 3: return Normalize({ u, -1.0f, v });
	case 4: return Normalize({ u, v, 1.0f });
	case 5: return Normalize({ -u, v, -1.0f });
	default:
		return { 0.0f, 0.0f, 1.0f };
	}
}

std::array<float, 4> SampleEquirectangular(const FloatImage& image, Float3 direction)
{
	if (image.width <= 0 || image.height <= 0 || image.pixels.empty()) {
		return { 0.0f, 0.0f, 0.0f, 1.0f };
	}

	constexpr float kPi = 3.14159265358979323846f;
	constexpr float kTwoPi = 6.28318530717958647692f;
	const float phi = std::atan2(direction.z, direction.x);
	const float theta = std::asin(std::clamp(direction.y, -1.0f, 1.0f));
	float u = 0.5f + phi / kTwoPi;
	float v = 0.5f - theta / kPi;
	u -= std::floor(u);
	v = std::clamp(v, 0.0f, 1.0f);

	const float x = u * static_cast<float>(std::max(image.width - 1, 1));
	const float y = v * static_cast<float>(std::max(image.height - 1, 1));
	const int x0 = static_cast<int>(std::floor(x));
	const int y0 = static_cast<int>(std::floor(y));
	const int x1 = (x0 + 1) % image.width;
	const int y1 = std::min(y0 + 1, image.height - 1);
	const float tx = x - static_cast<float>(x0);
	const float ty = y - static_cast<float>(y0);

	auto load_texel = [&image](int ix, int iy) {
		const size_t offset = (static_cast<size_t>(iy) * static_cast<size_t>(image.width) + static_cast<size_t>(ix)) * 4u;
		return std::array<float, 4>{
			image.pixels[offset + 0],
			image.pixels[offset + 1],
			image.pixels[offset + 2],
			image.pixels[offset + 3]
		};
	};

	const auto c00 = load_texel(x0, y0);
	const auto c10 = load_texel(x1, y0);
	const auto c01 = load_texel(x0, y1);
	const auto c11 = load_texel(x1, y1);

	std::array<float, 4> result{};
	for (size_t i = 0; i < result.size(); ++i) {
		const float top = c00[i] * (1.0f - tx) + c10[i] * tx;
		const float bottom = c01[i] * (1.0f - tx) + c11[i] * tx;
		result[i] = top * (1.0f - ty) + bottom * ty;
	}
	return result;
}

std::array<float, 4> ComputeAverageColor(const FloatImage& image)
{
	if (image.width <= 0 || image.height <= 0 || image.pixels.empty()) {
		return { 0.03f, 0.03f, 0.03f, 1.0f };
	}

	double weight_sum = 0.0;
	std::array<double, 4> accum{ 0.0, 0.0, 0.0, 0.0 };
	constexpr double kPi = 3.14159265358979323846;
	for (int y = 0; y < image.height; ++y) {
		const double v = (static_cast<double>(y) + 0.5) / static_cast<double>(image.height);
		const double theta = v * kPi;
		const double weight = std::sin(theta);
		for (int x = 0; x < image.width; ++x) {
			const size_t offset = (static_cast<size_t>(y) * static_cast<size_t>(image.width) + static_cast<size_t>(x)) * 4u;
			accum[0] += static_cast<double>(image.pixels[offset + 0]) * weight;
			accum[1] += static_cast<double>(image.pixels[offset + 1]) * weight;
			accum[2] += static_cast<double>(image.pixels[offset + 2]) * weight;
			accum[3] += static_cast<double>(image.pixels[offset + 3]) * weight;
			weight_sum += weight;
		}
	}

	if (weight_sum <= std::numeric_limits<double>::epsilon()) {
		return { 0.03f, 0.03f, 0.03f, 1.0f };
	}

	return {
		static_cast<float>(accum[0] / weight_sum),
		static_cast<float>(accum[1] / weight_sum),
		static_cast<float>(accum[2] / weight_sum),
		static_cast<float>(accum[3] / weight_sum)
	};
}

RhiTextureHandle CreateDx12SolidCubemap(DirectX12RhiDevice& device, const std::array<float, 4>& color)
{
	std::array<float, 24> pixels{};
	for (size_t face_index = 0; face_index < 6; ++face_index) {
		const size_t offset = face_index * 4u;
		pixels[offset + 0] = color[0];
		pixels[offset + 1] = color[1];
		pixels[offset + 2] = color[2];
		pixels[offset + 3] = color[3];
	}

	return device.CreateTexture(RhiTextureDesc{
		.dimension = RhiTextureDimension::TextureCube,
		.format = RhiTextureFormat::Rgb32Float,
		.width = 1,
		.height = 1
	}, pixels.data());
}

RhiTextureHandle CreateDx12CubemapFromEquirectangular(DirectX12RhiDevice& device, std::string_view path)
{
	const FloatImage source = LoadFloatImage(path);
	if (source.width <= 0 || source.height <= 0 || source.pixels.empty()) {
		return {};
	}

	const int face_size = std::clamp(std::min(source.width / 4, source.height / 2), 16, 512);
	std::vector<float> cubemap_pixels(static_cast<size_t>(face_size) * static_cast<size_t>(face_size) * 4u * 6u);
	for (uint32_t face_index = 0; face_index < 6; ++face_index) {
		for (int y = 0; y < face_size; ++y) {
			for (int x = 0; x < face_size; ++x) {
				const float u = (2.0f * (static_cast<float>(x) + 0.5f) / static_cast<float>(face_size)) - 1.0f;
				const float v = 1.0f - (2.0f * (static_cast<float>(y) + 0.5f) / static_cast<float>(face_size));
				const auto sample = SampleEquirectangular(source, GetCubemapDirection(face_index, u, v));
				const size_t offset =
					((static_cast<size_t>(face_index) * static_cast<size_t>(face_size) + static_cast<size_t>(y))
						* static_cast<size_t>(face_size)
						+ static_cast<size_t>(x)) * 4u;
				cubemap_pixels[offset + 0] = sample[0];
				cubemap_pixels[offset + 1] = sample[1];
				cubemap_pixels[offset + 2] = sample[2];
				cubemap_pixels[offset + 3] = sample[3];
			}
		}
	}

	return device.CreateTexture(RhiTextureDesc{
		.dimension = RhiTextureDimension::TextureCube,
		.format = RhiTextureFormat::Rgb32Float,
		.width = face_size,
		.height = face_size
	}, cubemap_pixels.data());
}

RhiTextureHandle CreateSolidDx12Cubemap(DirectX12RhiDevice& device, float r, float g, float b)
{
	return CreateDx12SolidCubemap(device, { r, g, b, 1.0f });
}

RhiTextureHandle CreateSolidDx12BrdfLut(DirectX12RhiDevice& device)
{
	const auto framebuffer = device.CreateFramebuffer(RhiFramebufferCreateDesc{
		.width = 1,
		.height = 1,
		.color_attachments = { RhiTextureFormat::Rg16Float }
	});
	if (!framebuffer.IsValid()) {
		return {};
	}

	device.SetClearColor(RhiClearColor{ 1.0f, 1.0f, 0.0f, 1.0f });
	device.BindFramebuffer(RhiFramebufferBindTarget::Draw, framebuffer);
	device.Clear(RhiClearFlags::Color);

	const RhiTextureHandle texture = device.TakeFramebufferTexture(framebuffer);
	device.DestroyFramebuffer(framebuffer);
	device.BindDefaultFramebuffer(RhiFramebufferBindTarget::Draw);
	return texture;
}

IblSetupResult BindDirectX12FallbackIblResources(const IblSetupContext& context, std::string_view reason)
{
	auto dx12_device = AsDirectX12RhiDevice(context.rhi_device);
	if (!dx12_device) {
		std::cerr << "IBL fallback skipped: DirectX12 RHI device is not available. Reason: " << reason << '\n';
		return { IblSetupStatus::Unavailable, false, std::string(reason) };
	}

	const RhiTextureHandle irradiance = CreateSolidDx12Cubemap(*dx12_device, 0.03f, 0.03f, 0.03f);
	const RhiTextureHandle prefilter = CreateSolidDx12Cubemap(*dx12_device, 0.03f, 0.03f, 0.03f);
	const RhiTextureHandle brdf_lut = CreateSolidDx12BrdfLut(*dx12_device);
	const RhiTextureHandle environment = CreateSolidDx12Cubemap(*dx12_device, 0.0f, 0.0f, 0.0f);
	if (!irradiance.IsValid() || !prefilter.IsValid() || !brdf_lut.IsValid() || !environment.IsValid()) {
		dx12_device->DestroyTexture(irradiance);
		dx12_device->DestroyTexture(prefilter);
		dx12_device->DestroyTexture(brdf_lut);
		dx12_device->DestroyTexture(environment);
		std::cerr << "IBL fallback skipped: DirectX12 placeholder resource creation failed. Reason: " << reason << '\n';
		return {
			IblSetupStatus::Unavailable,
			false,
			"DirectX12 placeholder IBL resource creation failed"
		};
	}

	std::cerr << "IBL fallback resources are used for DirectX12. Reason: " << reason << '\n';
	dx12_device->BindTextureUnit(kIblIrradianceBinding, irradiance);
	dx12_device->BindTextureUnit(kIblPrefilterBinding, prefilter);
	dx12_device->BindTextureUnit(kIblBrdfLutBinding, brdf_lut);
	dx12_device->BindTextureUnit(kIblEnvironmentBinding, environment);
	return { IblSetupStatus::FallbackReady, true, std::string(reason) };
}

IblSetupResult BindDirectX12ImageBasedLightingResources(const IblSetupContext& context)
{
	auto dx12_device = AsDirectX12RhiDevice(context.rhi_device);
	if (!dx12_device) {
		return { IblSetupStatus::Unavailable, false, "DirectX12 RHI device is not available" };
	}

	const FloatImage source_image = LoadFloatImage(context.hdr_path);
	if (source_image.width <= 0 || source_image.height <= 0 || source_image.pixels.empty()) {
		return BindDirectX12FallbackIblResources(
			context,
			std::string("HDR environment texture is missing or could not be loaded: ") + context.hdr_path);
	}

	const RhiTextureHandle environment = CreateDx12CubemapFromEquirectangular(*dx12_device, context.hdr_path);
	if (!environment.IsValid()) {
		return BindDirectX12FallbackIblResources(
			context,
			std::string("DirectX12 CPU equirectangular-to-cubemap conversion failed for: ") + context.hdr_path);
	}

	const auto average_color = ComputeAverageColor(source_image);
	const RhiTextureHandle irradiance = CreateDx12SolidCubemap(*dx12_device, average_color);
	const RhiTextureHandle brdf_lut = CreateSolidDx12BrdfLut(*dx12_device);
	if (!irradiance.IsValid() || !brdf_lut.IsValid()) {
		dx12_device->DestroyTexture(environment);
		dx12_device->DestroyTexture(irradiance);
		dx12_device->DestroyTexture(brdf_lut);
		return BindDirectX12FallbackIblResources(
			context,
			"DirectX12 partial IBL resource creation failed after environment cubemap conversion");
	}

	std::cerr << "DirectX12 IBL uses CPU-converted environment cubemap; irradiance uses average-color fallback and BRDF LUT remains placeholder until precompute is implemented.\n";
	dx12_device->BindTextureUnit(kIblIrradianceBinding, irradiance);
	dx12_device->BindTextureUnit(kIblPrefilterBinding, environment);
	dx12_device->BindTextureUnit(kIblBrdfLutBinding, brdf_lut);
	dx12_device->BindTextureUnit(kIblEnvironmentBinding, environment);
	return {
		IblSetupStatus::FallbackReady,
		true,
		"DirectX12 uses a CPU-converted environment cubemap while irradiance and BRDF LUT still use simplified fallbacks"
	};
}
#endif

} // namespace

const char* ToString(IblSetupStatus status)
{
	switch (status) {
	case IblSetupStatus::Ready:
		return "ready";
	case IblSetupStatus::FallbackReady:
		return "fallback-ready";
	case IblSetupStatus::Unavailable:
	default:
		return "unavailable";
	}
}

IblSetupResult SetupImageBasedLightingResources(const IblSetupContext& context)
{
	if (!context.rhi_device) {
		return { IblSetupStatus::Unavailable, false, "RHI device is not initialized" };
	}

	switch (context.rhi_device->GetBackendType()) {
	case RhiBackendType::OpenGL:
		if (context.matrices_ubo == nullptr ||
			context.sky_box_mesh == nullptr ||
			context.quad_mesh == nullptr ||
			context.equirectangular_to_cube_shader == nullptr ||
			context.irradiance_shader == nullptr ||
			context.prefilter_shader == nullptr ||
			context.brdf_lut_shader == nullptr) {
			return BindOpenGLFallbackIblResources(
				context,
				"IBL precompute mesh, shader, or UBO resources are not initialized");
		}
		return RunOpenGLIblPrecompute(context);
	case RhiBackendType::DirectX12:
#ifdef GSIM_RENDER_BACKEND_DX12
		return BindDirectX12ImageBasedLightingResources(context);
#else
		return {
			IblSetupStatus::Unavailable,
			false,
			"DirectX12 backend is not compiled into this build"
		};
#endif
	}

	return { IblSetupStatus::Unavailable, false, "Unknown RHI backend" };
}

} // namespace GComponent
