#pragma once
#include "../../Common.h"
#include "../RHIDefines.h"

namespace mira
{
	enum class MemoryType
	{
		Default,
		Upload,
		Readback
	};

	// We should rename this to something else
	enum class UsageIntent : u16
	{
		None = 1 << 0,				// All resources have a base SRV by default
		RenderTarget = 1 << 1,
		DepthStencil = 1 << 2,

		ShaderResource = 1 << 3,		// We should remove this

		UnorderedAccess = 1 << 4,
		DenyShaderResource = 1 << 5,
		CrossAdapter = 1 << 6,
		SimultaneousAccess = 1 << 7,
		RaytracingAS = 1 << 8,

		Constant = 1 << 9				// We should remove this
	};


	enum class ViewType : u8
	{
		None = 1 << 0,
		RenderTarget = 1 << 1,			// RTV
		DepthStencil = 1 << 2,			// DSV
		ShaderResource = 1 << 3,		// SRV
		UnorderedAccess = 1 << 4,		// UAV
		Constant = 1 << 5				// CBV
	};
	
	enum class DepthFormat
	{
		D32,
		D32_S8,
		D24_S8,
		D16
	};

	enum class ResourceFormat
	{
		Unknown,
		RGBA_32_FLOAT,
		RGBA_8_UNORM,
		
		D32_FLOAT,
		D32_FLOAT_S8X24_UINT,
		D24_UNORM_S8_UINT,
		D16_UNORM
	};

	enum class TextureType
	{
		Texture1D,
		Texture2D,
		Texture3D,
		Texture1D_Array,
		Texture2D_Array,
	};

	enum class ResourceState
	{
		Common,
		VertexAndConstantBuffer,
		IndexBuffer,
		RenderTarget,
		UnorderedAccess,
		DepthWrite,
		DepthRead,
		NonPixelShader,
		PixelShader,
		StreamOut,
		Indirect,
		CopyDst,
		CopySrc,
		ResolveDst,
		ResolveSrc,
		Raytracing_AS,
		ShadingRateSrc,
		GenericRead,
		AllShader,
		Present
	};

	struct BufferDesc
	{
		u32 size{ 0 };
		u32 alignment{ 0 };

		MemoryType memory_type{ MemoryType::Default };
		UsageIntent usage{ UsageIntent::None };

		// We should remove this
		// If buffer is to be accessed as SRV/UAV, user expected to work with the buffer with a single stride
		u32 stride{ 0 };
	};

	struct TextureDesc
	{
		u32 width{ 1 };
		u32 height{ 1 };
		u32 depth{ 1 };

		TextureType type{ TextureType::Texture2D };

		u32 alignment{ 0 };

		u32 mip_levels{ 0 };

		u32 sample_count{ 1 };
		u32 sample_quality{ 0 };

		std::array<float, 4> clear_color{ 0.f, 0.f, 0.f, 1.f };
		u8 stencil_clear{ 0 };
#ifdef USE_REVERSE_DEPTH
		float depth_clear{ 0.f };
#else
		float depth_clear{ 1.f };
#endif

		MemoryType memory_type{ MemoryType::Default };
		ResourceFormat format{ ResourceFormat::Unknown };
		UsageIntent usage{ UsageIntent::None };
	};





	inline UsageIntent operator|(UsageIntent a, UsageIntent b)
	{
		return static_cast<UsageIntent>(static_cast<u16>(a) | static_cast<u16>(b));
	}

	inline u16 operator&(UsageIntent a, UsageIntent b)
	{
		return static_cast<u16>(a) & static_cast<u16>(b);
	}

	inline ViewType operator|(ViewType a, ViewType b)
	{
		return static_cast<ViewType>(static_cast<u16>(a) | static_cast<u16>(b));
	}

	inline ViewType operator|=(ViewType& a, ViewType b)
	{
		a = a | b;
		return a;
	}

	inline u8 operator&(ViewType a, ViewType b)
	{
		return static_cast<u8>(a) & static_cast<u8>(b);
	}
}
