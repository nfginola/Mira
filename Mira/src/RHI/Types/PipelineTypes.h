#pragma once
#include "ShaderTypes.h"
#include "ResourceTypes.h"
#include "../RHIDefines.h"

namespace mira
{
	enum class PrimitiveTopology
	{
		None,
		
		PointList,

		TriangleList,
		TriangleList_Adj,

		TriangleStrip,
		TriangleStrip_Adj,

		LineList,
		LineList_Adj,

		LineStrip,
		LineStrip_Adj,

		PatchList			// To select the patch points --> Pass in PipelineStateDesc!
	};

	enum class FillMode
	{
		Wireframe,
		Solid
	};

	enum class CullMode
	{
		None,
		Front,
		Back
	};

	enum BlendFactor
	{
		ZERO,
		ONE,
		SRC_COLOR,
		INV_SRC_COLOR,
		SRC_ALPHA,
		INV_SRC_ALPHA,
		DEST_ALPHA,
		INV_DEST_ALPHA,
		DEST_COLOR ,
		INV_DEST_COLOR,
		SRC_ALPHA_SAT,
		BLEND_FACTOR,
		INV_BLEND_FACTOR ,
		SRC1_COLOR,
		INV_SRC1_COLOR,
		SRC1_ALPHA,
		INV_SRC1_ALPHA,
		ALPHA_FACTOR,
		INV_ALPHA_FACTOR
	};

	enum BlendOp
	{
		ADD,
		SUBTRACT,
		REV_SUBTRACT,
		MIN,
		MAX
	};

	enum class LogicOp
	{
		CLEAR,
		SET,
		COPY,
		COPY_INVERTED,
		NOOP,
		INVERT,
		AND,
		NAND,
		OR,
		NOR,
		XOR,
		EQUIV,
		AND_REVERSE,
		AND_INVERTED,
		OR_REVERSE,
		OR_INVERTED
	};

	enum class DepthWriteMask
	{
		Zero,
		All
	};

	enum class ComparisonFunc
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_EQUAL,
		ALWAYS
	};

	enum class StencilOp
	{
		KEEP,
		ZERO,
		REPLACE,
		INCR_SAT,
		DECR_SAT,
		INVERT,
		INCR,
		DECR
	};

	enum class ColorWriteEnable : u8
	{
		Red			= 1 << 0,
		Green		= 1 << 1,
		Blue		= 1 << 2,
		Alpha		= 1 << 3,
		All			= Red | Green | Blue | Alpha
	};

	struct DepthStencilOpDesc
	{
		StencilOp stencil_fail_op{ StencilOp::KEEP };
		StencilOp stencil_depth_fail_op{ StencilOp::KEEP };
		StencilOp stencil_pass_op{ StencilOp::KEEP };
		ComparisonFunc stencil_func{ ComparisonFunc::ALWAYS };
	};

	struct RenderTargetBlendDesc
	{
		bool blend_enabled{ false };
		bool logic_op_enabled{ false };
		
		BlendFactor src_blend{ BlendFactor::ONE };
		BlendFactor dst_blend{ BlendFactor::ZERO };
		BlendOp	blend_op{ BlendOp::ADD };

		BlendFactor src_blend_alpha{ BlendFactor::ONE };
		BlendFactor dst_blend_alpha{ BlendFactor::ZERO };
		BlendOp	blend_op_alpha{ BlendOp::ADD };

		LogicOp logic_op{ LogicOp::NOOP };
		ColorWriteEnable rt_write_mask{ ColorWriteEnable::All };
	};

	struct BlendDesc
	{
		bool alpha_to_coverage_enabled{ false };
		bool independent_blend_enabled{ false };
		
		RenderTargetBlendDesc rt_blends[8];
	};

	struct RasterizerStateDesc
	{
		FillMode fill_mode { FillMode::Solid };
		CullMode cull_mode { CullMode::Back };
		bool front_ccw{ false };

		i32 depth_bias{ 0 };
		float depth_bias_clamp{ 0.f };
		float slope_scaled_depth_bias{ 0.f };

		bool depth_clip_enabled{ true };
		bool multisample_enabled{ false };
		bool aa_line_enabled{ false };

		u32  forced_sample_count{ 0 };

		bool conservative_raster_enabled{ false };
	};

	struct DepthStencilDesc
	{
		bool depth_enabled{ false };
		DepthWriteMask depth_write_mask{ DepthWriteMask::All };

#ifdef USE_REVERSE_Z
		ComparisonFunc depth_func{ ComparisonFunc::GREATER };
#else
		ComparisonFunc depth_func{ ComparisonFunc::LESS };
#endif


		bool stencil_enabled{ false };
		u8 stencil_read_mask{ 0xFF };
		u8 stencil_write_mask{ 0xFF };

		DepthStencilOpDesc front_face{};
		DepthStencilOpDesc back_face{};
	};

	struct GraphicsPipelineDesc
	{
		const CompiledShader* vs{ nullptr };
		const CompiledShader* gs{ nullptr };
		const CompiledShader* ds{ nullptr };
		const CompiledShader* hs{ nullptr };
		const CompiledShader* ps{ nullptr };

		// Blend
		BlendDesc blend{};
		u32 sample_mask{ UINT_MAX };

		RasterizerStateDesc rasterizer{};
		DepthStencilDesc depth_stencil{};
		PrimitiveTopology topology{ PrimitiveTopology::TriangleList };
		
		u8 num_render_targets{ 0 };
		std::array<ResourceFormat, 8> rtv_formats{ ResourceFormat::Unknown };
		ResourceFormat dsv_format{ ResourceFormat::Unknown };

		// Multi-sample state
		u8 sample_count{ 1 };
		u8 sample_quality{ 0 };

		u32 num_control_patches{ 3 };
	};


	inline ColorWriteEnable operator|(ColorWriteEnable a, ColorWriteEnable b)
	{
		return static_cast<ColorWriteEnable>(static_cast<u16>(a) | static_cast<u16>(b));
	}

	inline u8 operator&(ColorWriteEnable a, ColorWriteEnable b)
	{
		return static_cast<u8>(a) & static_cast<u8>(b);
	}


}
