#pragma once
#include "../../Common.h"

namespace mira
{
	enum class ViewType
	{
		None,
		RenderTarget,			// RTV
		DepthStencil,			// DSV
		ShaderResource,			// SRV
		RaytracingAS,			// SRV (Raytracing Acceleration Structure)
		UnorderedAccess,		// UAV
		Constant,				// CBV
		Index					// IBV
	};

	// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageSubresourceRange.html
	// Borrowing Vulkan image view concept
	enum class TextureViewDimension
	{
		None,
		Texture1D,
		Texture1D_Array,

		Texture2D,
		Texture2D_MS,
		Texture2D_Array,
		Texture2D_MS_Array,

		Texture3D,
		TextureCube,
		TextureCube_Array,
	};

	struct TextureViewRange
	{
		TextureViewDimension dimension{ TextureViewDimension::None };
		ResourceFormat format{ ResourceFormat::Unknown };

		u32 base_mip_level{ 0 };
		u32 mip_levels{ (u32)-1 };

		u32 array_base{ 0 };
		u32 array_count{ 1 };

		float min_lod_clamp{ 0.f };

		// Flags to specify DS read-only
		bool depth_read_only{ false };
		bool stencil_read_only{ false };

		TextureViewRange() = default;
		TextureViewRange(TextureViewDimension dim_in, ResourceFormat format_in) : dimension(dim_in), format(format_in) {}

		TextureViewRange& set_array(u32 base, u32 count) { array_base = base; array_count = count; return *this; }
		TextureViewRange& set_mips(u32 base, u32 count) { base_mip_level = base; mip_levels = count; return *this; }
		TextureViewRange& set_min_lod_clamp(float value) { min_lod_clamp = value; return *this; }
		TextureViewRange& set_depth_read_only() { depth_read_only = true; return *this; }
		TextureViewRange& set_stencil_read_only() { stencil_read_only = true; return *this; }
	};

	struct BufferViewDesc
	{
		ViewType view{ ViewType::None };

		u32 offset{ 0 };
		u32 stride{ 0 };
		u32 count{ 1 };

		bool raw{ false };		// raw byte access

		BufferViewDesc(ViewType view_in, u32 offset_in, u32 stride_in, u32 count_in = 1, bool raw_in = false) :
			view(view_in), offset(offset_in), stride(stride_in), count(count_in), raw(raw_in) {}
	};

	struct TextureViewDesc
	{
		ViewType view{ ViewType::None };

		TextureViewRange range{};

		TextureViewDesc(ViewType view_in, TextureViewRange range_in) :
			view(view_in), range(range_in) {}
	};
}
