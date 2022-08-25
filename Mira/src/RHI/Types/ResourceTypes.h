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

	enum class UsageIntent : u16
	{
		None = 1 << 0,				
		RenderTarget = 1 << 1,
		DepthStencil = 1 << 2,

		UnorderedAccess = 1 << 3,
		DenyShaderResource = 1 << 4,
		CrossAdapter = 1 << 5,
		SimultaneousAccess = 1 << 6,
		RaytracingAS = 1 << 7,
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

	//inline ViewType operator|(ViewType a, ViewType b)
	//{
	//	return static_cast<ViewType>(static_cast<u16>(a) | static_cast<u16>(b));
	//}

	//inline ViewType operator|=(ViewType& a, ViewType b)
	//{
	//	a = a | b;
	//	return a;
	//}

	//inline u8 operator&(ViewType a, ViewType b)
	//{
	//	return static_cast<u8>(a) & static_cast<u8>(b);
	//}
}
