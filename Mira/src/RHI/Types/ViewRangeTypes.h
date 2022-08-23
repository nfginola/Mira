#pragma once
#include "../../Common.h"

namespace mira
{
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

		TextureViewRange(TextureViewDimension dim_in, ResourceFormat format_in) : dimension(dim_in), format(format_in) {}
	};


}
