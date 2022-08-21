#pragma once
#include "Types/PipelineTypes.h"

namespace mira
{
	class RasterizerBuilder
	{
	public:
		RasterizerBuilder() = default;
		~RasterizerBuilder() = default;

		RasterizerBuilder& set_fill_mode(FillMode mode) { m_desc.fill_mode = mode; return *this; };
		RasterizerBuilder& set_cull_mode(CullMode mode) { m_desc.cull_mode = mode; return *this; };
		RasterizerBuilder& set_front_is_ccw(bool enabled) { m_desc.front_ccw = enabled; return *this; };

		RasterizerBuilder& set_depth_clip_enabled(bool enabled) { m_desc.depth_clip_enabled = enabled; return *this; };
		RasterizerBuilder& set_multisample_enabled(bool enabled) { m_desc.multisample_enabled = enabled; return *this; };

		RasterizerBuilder& set_depth_bias(i32 bias) { m_desc.depth_bias = bias; return *this; };
		RasterizerBuilder& set_depth_bias_clamp(float clamp) { m_desc.depth_bias_clamp = clamp; return *this; };
		RasterizerBuilder& set_slope_scaled_depth_bias(float bias) { m_desc.slope_scaled_depth_bias = bias; return *this; };
		RasterizerBuilder& set_aa_line_enabled(bool enabled) { m_desc.aa_line_enabled = enabled; return *this; };
		RasterizerBuilder& set_forced_sample_count(u32 count) { m_desc.forced_sample_count = count; return *this; };
		RasterizerBuilder& set_conservative_raster_mode(bool mode) { m_desc.conservative_raster_enabled = mode; return *this; };

		operator RasterizerStateDesc() const { return m_desc; }
	private:
		RasterizerStateDesc m_desc{};
	};

	class BlendBuilder
	{
	public:
		BlendBuilder& set_alpha_to_coverage(bool enabled = false) { m_desc.alpha_to_coverage_enabled = enabled; return *this; };
		BlendBuilder& set_independent_blend(bool enabled = false) { m_desc.independent_blend_enabled = enabled; return *this; };

		// Ordering matters
		BlendBuilder& append_rt_blend(const RenderTargetBlendDesc& desc, bool enabled = false)
		{
			m_desc.rt_blends[m_blends_appended++] = desc;
			return *this;
		};

		operator BlendDesc() const { return m_desc; }

	private:
		u8 m_blends_appended{ 0 };
		BlendDesc m_desc{};
	};

	class DepthStencilBuilder
	{
	public:
		DepthStencilBuilder& set_depth_enabled(bool enabled) { m_desc.depth_enabled = enabled; return *this; }
		DepthStencilBuilder& set_depth_write_mask(DepthWriteMask mask) { m_desc.depth_write_mask = mask; return *this; }
		DepthStencilBuilder& set_depth_func(ComparisonFunc func) { m_desc.depth_func = func; return *this; }

		DepthStencilBuilder& set_stencil_enabled(bool enabled) { m_desc.stencil_enabled = enabled; return *this; }
		DepthStencilBuilder& set_stencil_read_mask(u8 mask) { m_desc.stencil_read_mask = mask; return *this; }
		DepthStencilBuilder& set_stencil_write_mask(u8 mask) { m_desc.stencil_write_mask = mask; return *this; }

		DepthStencilBuilder& set_stencil_op_front_face(const DepthStencilOpDesc& desc) { m_desc.front_face = desc; return *this; }
		DepthStencilBuilder& set_stencil_op_back_face(const DepthStencilOpDesc& desc) { m_desc.back_face = desc; return *this; }

		operator DepthStencilDesc() const { return m_desc; }

	private:
		DepthStencilDesc m_desc{};
	};


	// Helpers to fill in Pipeline Desc
	class GraphicsPipelineBuilder
	{
	public:
		GraphicsPipelineBuilder();

		GraphicsPipelineBuilder& set_topology(PrimitiveTopology topology) { m_desc.topology = topology; return *this; }
		GraphicsPipelineBuilder& set_sample_mask(u32 mask) { m_desc.sample_mask = mask; return *this; }
		GraphicsPipelineBuilder& set_multisample(u32 count, u32 quality) { m_desc.sample_count = count; m_desc.sample_quality = quality; return *this; };
		GraphicsPipelineBuilder& set_depth_format(DepthFormat format);

		GraphicsPipelineBuilder& set_shader(const CompiledShader* shader);

		// Ordering matters
		GraphicsPipelineBuilder& append_rt_format(ResourceFormat format);

		GraphicsPipelineBuilder& set_rasterizer(RasterizerBuilder& builder) { m_desc.rasterizer = builder; return *this; }
		GraphicsPipelineBuilder& set_blend(BlendBuilder& builder) { m_desc.blend = builder; return *this; }
		GraphicsPipelineBuilder& set_depth_stencil(DepthStencilBuilder& builder) { m_desc.depth_stencil = builder; return *this; };

		const GraphicsPipelineDesc& build();

	private:
		GraphicsPipelineDesc m_desc;

	};

}
