#include "StructTranslator_DX12.h"

namespace mira
{

	D3D12_HEAP_TYPE to_internal(MemoryType type)
	{
		switch (type)
		{
		case MemoryType::Default:
			return D3D12_HEAP_TYPE_DEFAULT;
		case MemoryType::Upload:
			return D3D12_HEAP_TYPE_UPLOAD;
		case MemoryType::Readback:
			return D3D12_HEAP_TYPE_READBACK;
		default:
			assert(false);
		}
		assert(false);
		return D3D12_HEAP_TYPE_CUSTOM;

	}

	D3D12_RESOURCE_FLAGS to_internal(UsageIntent usage)
	{
		D3D12_RESOURCE_FLAGS flags{ D3D12_RESOURCE_FLAG_NONE };
		if (usage & UsageIntent::RenderTarget)
			flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		if (usage & UsageIntent::DepthStencil)
			flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		if (usage & UsageIntent::UnorderedAccess)
			flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		if (usage & UsageIntent::DenyShaderResource)
			flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		if (usage & UsageIntent::CrossAdapter)
			flags |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
		if (usage & UsageIntent::SimultaneousAccess)
			flags |= D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
		if (usage & UsageIntent::RaytracingAS)
			flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;

		return flags;
	}

	DXGI_FORMAT to_internal(ResourceFormat format)
	{
		switch (format)
		{
		case ResourceFormat::Unknown:
			return DXGI_FORMAT_UNKNOWN;
		case ResourceFormat::RGBA_32_FLOAT:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case ResourceFormat::RGBA_8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case ResourceFormat::D32_FLOAT:
			return DXGI_FORMAT_D32_FLOAT;
		case ResourceFormat::D32_FLOAT_S8X24_UINT:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case ResourceFormat::D24_UNORM_S8_UINT:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case ResourceFormat::D16_UNORM:
			return DXGI_FORMAT_D16_UNORM;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
		return DXGI_FORMAT_UNKNOWN;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC to_internal(GraphicsPipelineDesc desc, ID3D12RootSignature* rootsig)
	{
		// Limited support, for now
		D3D12_GRAPHICS_PIPELINE_STATE_DESC api_desc{};
		api_desc.pRootSignature = rootsig;
		api_desc.VS = to_internal(desc.vs);
		api_desc.PS = to_internal(desc.ps);
		api_desc.RasterizerState = to_internal(desc.rasterizer);
		api_desc.DepthStencilState = to_internal(desc.depth_stencil);

		api_desc.SampleMask = desc.sample_mask;
		api_desc.BlendState = to_internal(desc.blend);

		api_desc.SampleDesc.Count = desc.sample_count;
		api_desc.SampleDesc.Quality = desc.sample_quality;

		api_desc.PrimitiveTopologyType = to_internal(desc.topology);

		api_desc.DSVFormat = to_internal(desc.dsv_format);
		api_desc.NumRenderTargets = desc.num_render_targets;
		for (u32 i = 0; i < desc.num_render_targets; ++i)
			api_desc.RTVFormats[i] = to_internal(desc.rtv_formats[i]);

		api_desc.StreamOutput = {};
		api_desc.InputLayout = {};
		api_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		api_desc.NodeMask = 0;
		api_desc.CachedPSO = {};
		api_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		return api_desc;
	}

	D3D12_SHADER_BYTECODE to_internal(const CompiledShader* compiled_shader)
	{
		D3D12_SHADER_BYTECODE bc{};
		bc.pShaderBytecode = compiled_shader->blob.data();
		bc.BytecodeLength = compiled_shader->blob.size();
		return bc;
	}

	D3D12_RASTERIZER_DESC to_internal(const RasterizerStateDesc& desc)
	{
		D3D12_RASTERIZER_DESC api_desc{};
		api_desc.FillMode = to_internal(desc.fill_mode);
		api_desc.CullMode = to_internal(desc.cull_mode);
		api_desc.FrontCounterClockwise = desc.front_ccw;
		api_desc.DepthBias = desc.depth_bias;
		api_desc.DepthBiasClamp = desc.depth_bias_clamp;
		api_desc.SlopeScaledDepthBias = desc.slope_scaled_depth_bias;
		api_desc.DepthClipEnable = desc.depth_clip_enabled;
		api_desc.MultisampleEnable = desc.multisample_enabled;
		api_desc.AntialiasedLineEnable = desc.aa_line_enabled;
		api_desc.ForcedSampleCount = desc.forced_sample_count;
		api_desc.ConservativeRaster = desc.conservative_raster_enabled ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		return api_desc;
	}

	D3D12_DEPTH_STENCIL_DESC to_internal(const DepthStencilDesc& desc)
	{
		D3D12_DEPTH_STENCIL_DESC api_desc{};
		api_desc.DepthEnable = desc.depth_enabled;
		api_desc.DepthWriteMask = to_internal(desc.depth_write_mask);
		api_desc.DepthFunc = to_internal(desc.depth_func);
		api_desc.StencilEnable = desc.stencil_enabled;
		api_desc.StencilReadMask = desc.stencil_read_mask;
		api_desc.StencilWriteMask = desc.stencil_write_mask;
		api_desc.FrontFace = to_internal(desc.front_face);
		api_desc.BackFace = to_internal(desc.back_face);

		return api_desc;
	}

	D3D12_BLEND_DESC to_internal(const BlendDesc& desc)
	{
		D3D12_BLEND_DESC api_desc{};
		api_desc.AlphaToCoverageEnable = desc.alpha_to_coverage_enabled;
		api_desc.IndependentBlendEnable = desc.independent_blend_enabled;
		for (u32 i = 0; i < 8; ++i)
			api_desc.RenderTarget[i] = to_internal(desc.rt_blends[i]);

		return api_desc;
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE to_internal(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::None:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		case PrimitiveTopology::PointList:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case PrimitiveTopology::LineList:
		case PrimitiveTopology::LineList_Adj:
		case PrimitiveTopology::LineStrip:
		case PrimitiveTopology::LineStrip_Adj:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case PrimitiveTopology::TriangleList:
		case PrimitiveTopology::TriangleList_Adj:
		case PrimitiveTopology::TriangleStrip:
		case PrimitiveTopology::TriangleStrip_Adj:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case PrimitiveTopology::PatchList:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		default:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}
	}

	D3D_PRIMITIVE_TOPOLOGY to_internal_topology(PrimitiveTopology topology, u8 patch_control_points)
	{
		switch (topology)
		{
		case PrimitiveTopology::None:
			return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		case PrimitiveTopology::PointList:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case PrimitiveTopology::LineList:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case PrimitiveTopology::LineList_Adj:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
		case PrimitiveTopology::LineStrip:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case PrimitiveTopology::LineStrip_Adj:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
		case PrimitiveTopology::TriangleList:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case PrimitiveTopology::TriangleList_Adj:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
		case PrimitiveTopology::TriangleStrip:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case PrimitiveTopology::TriangleStrip_Adj:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
		case PrimitiveTopology::PatchList:
		{
			assert(patch_control_points > 1 && patch_control_points <= 32);
			return D3D_PRIMITIVE_TOPOLOGY(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + (patch_control_points - 1));
		}
		default:
			return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}

	D3D12_FILL_MODE to_internal(FillMode mode)
	{
		switch (mode)
		{
		case FillMode::Wireframe:
			return D3D12_FILL_MODE_WIREFRAME;
		case FillMode::Solid:
			return D3D12_FILL_MODE_SOLID;
		default:
			return D3D12_FILL_MODE_SOLID;
		}
	}

	D3D12_CULL_MODE to_internal(CullMode mode)
	{
		switch (mode)
		{
		case CullMode::None:
			return D3D12_CULL_MODE_NONE;
		case CullMode::Front:
			return D3D12_CULL_MODE_FRONT;
		case CullMode::Back:
			return D3D12_CULL_MODE_BACK;
		default:
			return D3D12_CULL_MODE_BACK;
		}
	}

	D3D12_DEPTH_WRITE_MASK to_internal(DepthWriteMask mask)
	{
		switch (mask)
		{
		case DepthWriteMask::Zero:
			return D3D12_DEPTH_WRITE_MASK_ZERO;
		case DepthWriteMask::All:
			return D3D12_DEPTH_WRITE_MASK_ALL;
		default:
			return D3D12_DEPTH_WRITE_MASK_ALL;
		}
	}

	D3D12_COMPARISON_FUNC to_internal(ComparisonFunc func)
	{
		switch (func)
		{
		case ComparisonFunc::NEVER:
			return D3D12_COMPARISON_FUNC_NEVER;
		case ComparisonFunc::LESS:
			return D3D12_COMPARISON_FUNC_LESS;
		case ComparisonFunc::EQUAL:
			return D3D12_COMPARISON_FUNC_EQUAL;
		case ComparisonFunc::LESS_EQUAL:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case ComparisonFunc::GREATER:
			return D3D12_COMPARISON_FUNC_GREATER;
		case ComparisonFunc::NOT_EQUAL:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case ComparisonFunc::GREATER_EQUAL:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case ComparisonFunc::ALWAYS:
			return D3D12_COMPARISON_FUNC_ALWAYS;
		default:
			return D3D12_COMPARISON_FUNC_NEVER;
		}
	}

	D3D12_DEPTH_STENCILOP_DESC to_internal(const DepthStencilOpDesc& desc)
	{
		D3D12_DEPTH_STENCILOP_DESC api_desc{};
		api_desc.StencilFailOp = to_internal(desc.stencil_depth_fail_op);
		api_desc.StencilDepthFailOp = to_internal(desc.stencil_depth_fail_op);
		api_desc.StencilPassOp = to_internal(desc.stencil_pass_op);
		api_desc.StencilFunc = to_internal(desc.stencil_func);

		return api_desc;
	}

	D3D12_STENCIL_OP to_internal(StencilOp op)
	{
		switch (op)
		{
		case StencilOp::KEEP:
			return D3D12_STENCIL_OP_KEEP;
		case StencilOp::ZERO:
			return D3D12_STENCIL_OP_ZERO;
		case StencilOp::REPLACE:
			return D3D12_STENCIL_OP_REPLACE;
		case StencilOp::INCR_SAT:
			return D3D12_STENCIL_OP_INCR_SAT;
		case StencilOp::DECR_SAT:
			return D3D12_STENCIL_OP_DECR_SAT;
		case StencilOp::INVERT:
			return D3D12_STENCIL_OP_INVERT;
		case StencilOp::INCR:
			return D3D12_STENCIL_OP_INCR;
		case StencilOp::DECR:
			return D3D12_STENCIL_OP_DECR;
		default:
			return D3D12_STENCIL_OP_KEEP;
		}
	}

	D3D12_RENDER_TARGET_BLEND_DESC to_internal(const RenderTargetBlendDesc& desc)
	{
		D3D12_RENDER_TARGET_BLEND_DESC api_desc{};
		api_desc.BlendEnable = desc.blend_enabled;
		api_desc.LogicOpEnable = desc.logic_op_enabled;

		api_desc.SrcBlend = to_internal(desc.src_blend);
		api_desc.DestBlend = to_internal(desc.dst_blend);
		api_desc.BlendOp = to_internal(desc.blend_op);

		api_desc.SrcBlendAlpha = to_internal(desc.src_blend_alpha);
		api_desc.DestBlendAlpha = to_internal(desc.dst_blend_alpha);
		api_desc.BlendOpAlpha = to_internal(desc.blend_op_alpha);

		api_desc.LogicOp = to_internal(desc.logic_op);
		api_desc.RenderTargetWriteMask = to_internal(desc.rt_write_mask);

		return api_desc;
	}

	D3D12_BLEND to_internal(BlendFactor fac)
	{
		switch (fac)
		{
		case ZERO:
			return D3D12_BLEND_ZERO;
		case ONE:
			return D3D12_BLEND_ONE;
		case SRC_COLOR:
			return D3D12_BLEND_SRC_COLOR;
		case INV_SRC_COLOR:
			return D3D12_BLEND_INV_SRC_COLOR;
		case SRC_ALPHA:
			return D3D12_BLEND_SRC_ALPHA;
		case INV_SRC_ALPHA:
			return D3D12_BLEND_INV_SRC_ALPHA;
		case DEST_ALPHA:
			return D3D12_BLEND_DEST_ALPHA;
		case INV_DEST_ALPHA:
			return D3D12_BLEND_INV_DEST_ALPHA;
		case DEST_COLOR:
			return D3D12_BLEND_DEST_COLOR;
		case INV_DEST_COLOR:
			return D3D12_BLEND_INV_DEST_COLOR;
		case SRC_ALPHA_SAT:
			return D3D12_BLEND_SRC_ALPHA_SAT;
		case BLEND_FACTOR:
			return D3D12_BLEND_BLEND_FACTOR;
		case INV_BLEND_FACTOR:
			return D3D12_BLEND_INV_BLEND_FACTOR;
		case SRC1_COLOR:
			return D3D12_BLEND_SRC1_COLOR;
		case INV_SRC1_COLOR:
			return D3D12_BLEND_INV_SRC1_COLOR;
		case SRC1_ALPHA:
			return D3D12_BLEND_SRC1_ALPHA;
		case INV_SRC1_ALPHA:
			return D3D12_BLEND_INV_SRC1_ALPHA;
		case ALPHA_FACTOR:
			return D3D12_BLEND_ALPHA_FACTOR;
		case INV_ALPHA_FACTOR:
			return D3D12_BLEND_INV_ALPHA_FACTOR;
		default:
			return D3D12_BLEND_ZERO;
		}
	}

	D3D12_BLEND_OP to_internal(BlendOp op)
	{
		switch (op)
		{
		case BlendOp::ADD:
			return D3D12_BLEND_OP_ADD;
		case BlendOp::SUBTRACT:
			return D3D12_BLEND_OP_SUBTRACT;
		case BlendOp::REV_SUBTRACT:
			return D3D12_BLEND_OP_REV_SUBTRACT;
		case BlendOp::MIN:
			return D3D12_BLEND_OP_MIN;
		case BlendOp::MAX:
			return D3D12_BLEND_OP_MAX;
		default:
			return D3D12_BLEND_OP_ADD;
		}
	}

	D3D12_LOGIC_OP to_internal(LogicOp op)
	{
		switch (op)
		{
		case LogicOp::CLEAR:
			return D3D12_LOGIC_OP_CLEAR;
		case LogicOp::SET:
			return D3D12_LOGIC_OP_SET;
		case LogicOp::COPY:
			return D3D12_LOGIC_OP_COPY;
		case LogicOp::COPY_INVERTED:
			return D3D12_LOGIC_OP_COPY_INVERTED;
		case LogicOp::NOOP:
			return D3D12_LOGIC_OP_NOOP;
		case LogicOp::INVERT:
			return D3D12_LOGIC_OP_INVERT;
		case LogicOp::AND:
			return D3D12_LOGIC_OP_AND;
		case LogicOp::NAND:
			return D3D12_LOGIC_OP_NAND;
		case LogicOp::OR:
			return D3D12_LOGIC_OP_OR;
		case LogicOp::NOR:
			return D3D12_LOGIC_OP_NOR;
		case LogicOp::XOR:
			return D3D12_LOGIC_OP_XOR;
		case LogicOp::EQUIV:
			return D3D12_LOGIC_OP_EQUIV;
		case LogicOp::AND_REVERSE:
			return D3D12_LOGIC_OP_AND_REVERSE;
		case LogicOp::AND_INVERTED:
			return D3D12_LOGIC_OP_AND_INVERTED;
		case LogicOp::OR_REVERSE:
			return D3D12_LOGIC_OP_OR_REVERSE;
		case LogicOp::OR_INVERTED:
			return D3D12_LOGIC_OP_OR_INVERTED;
		default:
			return D3D12_LOGIC_OP_CLEAR;
		}

	}

	u8 to_internal(ColorWriteEnable mask)
	{
		u8 api_mask{ 0 };

		if (mask & ColorWriteEnable::Red)
			api_mask |= D3D12_COLOR_WRITE_ENABLE_RED;
		if (mask & ColorWriteEnable::Green)
			api_mask |= D3D12_COLOR_WRITE_ENABLE_GREEN;
		if (mask & ColorWriteEnable::Blue)
			api_mask |= D3D12_COLOR_WRITE_ENABLE_BLUE;
		if (mask & ColorWriteEnable::All)
			api_mask |= D3D12_COLOR_WRITE_ENABLE_ALL;

		return api_mask;
	}

	D3D12_RESOURCE_DIMENSION to_internal(TextureType type)
	{
		switch (type)
		{
			case TextureType::Texture1D:
				return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
			case TextureType::Texture2D:
				return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			case TextureType::Texture3D:
				return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		default:
			return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		}
	}

	D3D12_RTV_DIMENSION to_internal_rtv(TextureViewDimension type)
	{
		switch (type)
		{
		case TextureViewDimension::Texture1D:
			return D3D12_RTV_DIMENSION_TEXTURE1D;
		case TextureViewDimension::Texture1D_Array:
			return D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
		case TextureViewDimension::Texture2D:
			return D3D12_RTV_DIMENSION_TEXTURE2D;
		case TextureViewDimension::Texture2D_MS:
			return D3D12_RTV_DIMENSION_TEXTURE2DMS;
		case TextureViewDimension::Texture2D_MS_Array:
			return D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
		case TextureViewDimension::Texture2D_Array:
			return D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		case TextureViewDimension::Texture3D:
			return D3D12_RTV_DIMENSION_TEXTURE3D;
		default:
			return D3D12_RTV_DIMENSION_UNKNOWN;
		}
	}

	D3D12_DSV_DIMENSION to_internal_dsv(TextureViewDimension type)
	{
		switch (type)
		{
		case TextureViewDimension::Texture1D:
			return D3D12_DSV_DIMENSION_TEXTURE1D;
		case TextureViewDimension::Texture1D_Array:
			return D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
		case TextureViewDimension::Texture2D:
			return D3D12_DSV_DIMENSION_TEXTURE2D;
		case TextureViewDimension::Texture2D_Array:
			return D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		case TextureViewDimension::Texture2D_MS:
			return D3D12_DSV_DIMENSION_TEXTURE2DMS;
		case TextureViewDimension::Texture2D_MS_Array:
			return D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
		default:
			return D3D12_DSV_DIMENSION_UNKNOWN;
		}
	}

	D3D12_SRV_DIMENSION to_internal_srv(TextureViewDimension type)
	{
		switch (type)
		{
		case TextureViewDimension::Texture1D:
			return D3D12_SRV_DIMENSION_TEXTURE1D;
		case TextureViewDimension::Texture1D_Array:
			return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
		case TextureViewDimension::Texture2D:
			return D3D12_SRV_DIMENSION_TEXTURE2D;
		case TextureViewDimension::Texture2D_Array:
			return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		case TextureViewDimension::Texture3D:
			return D3D12_SRV_DIMENSION_TEXTURE3D;
		case TextureViewDimension::TextureCube:
			return D3D12_SRV_DIMENSION_TEXTURECUBE;
		case TextureViewDimension::TextureCube_Array:
			return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
		default:
			return D3D12_SRV_DIMENSION_UNKNOWN;
		}
	}

	D3D12_UAV_DIMENSION to_internal_uav(TextureViewDimension type)
	{
		switch (type)
		{
		case TextureViewDimension::Texture1D:
			return D3D12_UAV_DIMENSION_TEXTURE1D;
		case TextureViewDimension::Texture1D_Array:
			return D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
		case TextureViewDimension::Texture2D:
			return D3D12_UAV_DIMENSION_TEXTURE2D;
		case TextureViewDimension::Texture2D_Array:
			return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		case TextureViewDimension::Texture3D:
			return D3D12_UAV_DIMENSION_TEXTURE3D;
		default:
			return D3D12_UAV_DIMENSION_UNKNOWN;
		}
	}

	D3D12_RENDER_PASS_RENDER_TARGET_DESC to_internal(const RenderPassTargetDesc& desc)
	{
		D3D12_RENDER_PASS_RENDER_TARGET_DESC api_desc{};
		api_desc.BeginningAccess.Type = to_internal(desc.begin_access);
		api_desc.EndingAccess.Type = to_internal(desc.end_access);
		return api_desc;	// calling api still needs to finish the begin/end access resources (clear/resources)
	}

	D3D12_RENDER_PASS_DEPTH_STENCIL_DESC to_internal(const RenderPassDepthStencilTargetDesc& desc)
	{
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC api_desc{};
		api_desc.DepthBeginningAccess.Type = to_internal(desc.begin_depth_access);
		api_desc.DepthEndingAccess.Type = to_internal(desc.end_depth_access);
		api_desc.StencilBeginningAccess.Type = to_internal(desc.begin_stencil_access);
		api_desc.StencilEndingAccess.Type = to_internal(desc.end_stencil_access);
		return api_desc;	// calling api still needs to finish the begin/end access resources (clear/resources)
	}

	D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE to_internal(RenderPassBeginAccessType access)
	{
		switch (access)
		{
		case RenderPassBeginAccessType::Clear:
			return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
		case RenderPassBeginAccessType::Discard:
			return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
		case RenderPassBeginAccessType::Preserve:
			return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
		default:
			return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
		}
	}

	D3D12_RENDER_PASS_ENDING_ACCESS_TYPE to_internal(RenderPassEndingAccessType access)
	{
		switch (access)
		{
		case RenderPassEndingAccessType::Discard:
			return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
		case RenderPassEndingAccessType::Preserve:
			return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
		case RenderPassEndingAccessType::Resolve:
			return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
		default:
			return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
		}
	}

	D3D12_RENDER_PASS_FLAGS to_internal(RenderPassFlag flags)
	{
		switch (flags)
		{
		case RenderPassFlag::AllowUnorderedAccessWrites:
			return D3D12_RENDER_PASS_FLAG_ALLOW_UAV_WRITES;
		default:
			return D3D12_RENDER_PASS_FLAG_NONE;
		}
	}

	D3D12_RESOURCE_STATES to_internal(ResourceState state)
	{
		switch (state)
		{
			case ResourceState::Common:
				return D3D12_RESOURCE_STATE_COMMON;
			case ResourceState::VertexAndConstantBuffer:
				return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			case ResourceState::IndexBuffer:
				return D3D12_RESOURCE_STATE_INDEX_BUFFER;
			case ResourceState::RenderTarget:
				return D3D12_RESOURCE_STATE_RENDER_TARGET;
			case ResourceState::UnorderedAccess:
				return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			case ResourceState::DepthWrite:
				return D3D12_RESOURCE_STATE_DEPTH_WRITE;
			case ResourceState::DepthRead:
				return D3D12_RESOURCE_STATE_DEPTH_READ;
			case ResourceState::NonPixelShader:
				return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			case ResourceState::PixelShader:
				return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			case ResourceState::StreamOut:
				return D3D12_RESOURCE_STATE_STREAM_OUT;
			case ResourceState::Indirect:
				return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
			case ResourceState::CopyDst:
				return D3D12_RESOURCE_STATE_COPY_DEST;
			case ResourceState::CopySrc:
				return D3D12_RESOURCE_STATE_COPY_SOURCE;
			case ResourceState::ResolveDst:
				return D3D12_RESOURCE_STATE_RESOLVE_DEST;
			case ResourceState::ResolveSrc:
				return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
			case ResourceState::Raytracing_AS:
				return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
			case ResourceState::ShadingRateSrc:
				return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
			case ResourceState::GenericRead:
				return D3D12_RESOURCE_STATE_GENERIC_READ;
			case ResourceState::AllShader:
				return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
			case ResourceState::Present:
				return D3D12_RESOURCE_STATE_PRESENT;
			default:
				return D3D12_RESOURCE_STATE_COMMON;
		}
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC to_srv(const TextureViewRange& range)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Format = to_internal(range.format);
		desc.ViewDimension = to_internal_srv(range.dimension);
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		switch (range.dimension)
		{
		case TextureViewDimension::Texture1D:
		{
			desc.Texture1D.MostDetailedMip = range.base_mip_level;
			desc.Texture1D.MipLevels = range.mip_levels;
			desc.Texture1D.ResourceMinLODClamp = range.min_lod_clamp;
			break;
		}
		case TextureViewDimension::Texture1D_Array:
		{
			desc.Texture1DArray.MostDetailedMip = range.base_mip_level;
			desc.Texture1DArray.MipLevels = range.mip_levels;
			desc.Texture1DArray.ResourceMinLODClamp = range.min_lod_clamp;

			desc.Texture1DArray.ArraySize = range.array_count;
			desc.Texture1DArray.FirstArraySlice = range.array_base;		
			break;
		}
		case TextureViewDimension::Texture2D:
		{
			desc.Texture2D.MostDetailedMip = range.base_mip_level;
			desc.Texture2D.MipLevels = range.mip_levels;
			desc.Texture2D.ResourceMinLODClamp = range.min_lod_clamp;

			desc.Texture2D.PlaneSlice = range.array_base;
			assert(range.array_count == 1);

			break;
		}
		case TextureViewDimension::Texture2D_Array:
		{
			desc.Texture2DArray.MostDetailedMip = range.base_mip_level;
			desc.Texture2DArray.MipLevels = range.mip_levels;
			desc.Texture2DArray.ResourceMinLODClamp = range.min_lod_clamp;

			desc.Texture2DArray.ArraySize = range.array_count;
			desc.Texture2DArray.FirstArraySlice = range.array_base;
			desc.Texture2DArray.PlaneSlice = 0;

			break;
		}
		case TextureViewDimension::Texture2D_MS:
		{
			desc.Texture2DMS.UnusedField_NothingToDefine = 0;
			break;
		}
		case TextureViewDimension::Texture2D_MS_Array:
		{
			desc.Texture2DMSArray.FirstArraySlice = range.array_base;
			desc.Texture2DMSArray.ArraySize = range.array_count;

			break;
		}
		case TextureViewDimension::Texture3D:
		{
			desc.Texture3D.MostDetailedMip = range.base_mip_level;
			desc.Texture3D.MipLevels = range.mip_levels;
			desc.Texture3D.ResourceMinLODClamp = range.min_lod_clamp;

			break;
		}
		case TextureViewDimension::TextureCube:
		{
			desc.TextureCube.MostDetailedMip = range.base_mip_level;
			desc.TextureCube.MipLevels = range.mip_levels;
			desc.TextureCube.ResourceMinLODClamp = range.min_lod_clamp;
			break;
		}
		case TextureViewDimension::TextureCube_Array:
		{
			desc.TextureCubeArray.MostDetailedMip = range.base_mip_level;
			desc.TextureCubeArray.MipLevels = range.mip_levels;
			desc.TextureCubeArray.ResourceMinLODClamp = range.min_lod_clamp;

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageSubresourceRange.html
			// Above is the only doc I can find for what "First2DArrayFace" means in D12 ...
			desc.TextureCubeArray.NumCubes = range.array_count;
			desc.TextureCubeArray.First2DArrayFace = range.array_base;		// Represent base WITHIN a texture cube! (array_base + i): where i % 6

			break;
		}
		default:
			assert(false);
		}

		return desc;
	}

	D3D12_RENDER_TARGET_VIEW_DESC to_rtv(const TextureViewRange& range)
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = to_internal(range.format);
		desc.ViewDimension = to_internal_rtv(range.dimension);
		switch (range.dimension)
		{
		case TextureViewDimension::Texture1D:
		{
			desc.Texture1D.MipSlice = range.array_base;

			assert(range.array_count == 1);
			break;
		}
		case TextureViewDimension::Texture1D_Array:
		{
			desc.Texture1DArray.FirstArraySlice = range.array_base;
			desc.Texture1DArray.ArraySize = range.array_count;
			desc.Texture1DArray.MipSlice = range.base_mip_level;
			assert(range.mip_levels == 1);

			break;
		}
		case TextureViewDimension::Texture2D:
		{
			desc.Texture2D.MipSlice = range.array_base;
			desc.Texture2D.PlaneSlice = 0;

			assert(range.array_count == 1);
			break;
		}
		case TextureViewDimension::Texture2D_Array:
		{
			desc.Texture2DArray.FirstArraySlice = range.array_base;
			desc.Texture2DArray.ArraySize = range.array_count;
			desc.Texture2DArray.MipSlice = range.base_mip_level;

			desc.Texture2DArray.PlaneSlice = 0;

			assert(range.mip_levels == 1);

			break;
		}
		case TextureViewDimension::Texture2D_MS:
		{
			desc.Texture2DMS.UnusedField_NothingToDefine = 0;
			break;
		}
		case TextureViewDimension::Texture2D_MS_Array:
		{
			desc.Texture2DMSArray.FirstArraySlice = range.array_base;
			desc.Texture2DMSArray.ArraySize = range.array_count;
			break;
		}
		case TextureViewDimension::Texture3D:
		{
			desc.Texture3D.MipSlice = range.base_mip_level;

			desc.Texture3D.FirstWSlice = range.array_base;
			desc.Texture3D.WSize = range.array_count;

			assert(range.mip_levels == 1);

			break;
		}
		default:
			assert(false);
		}

		return desc;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC to_dsv(const TextureViewRange& range)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
		desc.Format = to_internal(range.format);
		desc.ViewDimension = to_internal_dsv(range.dimension);
		desc.Flags = D3D12_DSV_FLAG_NONE;
		if (range.depth_read_only)
			desc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
		if (range.stencil_read_only)
			desc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;

		switch (range.dimension)
		{
		case TextureViewDimension::Texture1D:
		{
			desc.Texture1D.MipSlice = range.base_mip_level;
			
			assert(range.mip_levels == 1);

			break;
		}
		case TextureViewDimension::Texture1D_Array:
		{
			desc.Texture1DArray.MipSlice = range.base_mip_level;
			desc.Texture1DArray.FirstArraySlice = range.array_base;
			desc.Texture1DArray.ArraySize = range.array_count;

			assert(range.mip_levels == 1);

			break;
		}
		case TextureViewDimension::Texture2D:
		{
			desc.Texture2D.MipSlice = range.base_mip_level;

			assert(range.mip_levels == 1);

			break;
		}
		case TextureViewDimension::Texture2D_Array:
		{
			desc.Texture2DArray.MipSlice = range.base_mip_level;
			desc.Texture2DArray.FirstArraySlice = range.array_base;
			desc.Texture2DArray.ArraySize = range.array_count;

			assert(range.mip_levels == 1);

			break;
		}
		case TextureViewDimension::Texture2D_MS:
		{
			desc.Texture2DMS.UnusedField_NothingToDefine = 0;

			break;
		}
		case TextureViewDimension::Texture2D_MS_Array:
		{
			desc.Texture2DMSArray.FirstArraySlice = range.array_base;
			desc.Texture2DMSArray.ArraySize = range.array_count;
			break;
		}
			
		default:
			assert(false);
		}

		return desc;
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC to_uav(const TextureViewRange& range)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.Format = to_internal(range.format);
		desc.ViewDimension = to_internal_uav(range.dimension);

		assert(range.mip_levels == 1);

		switch (range.dimension)
		{
		case TextureViewDimension::Texture1D:
		{
			desc.Texture1D.MipSlice = range.base_mip_level;

			break;
		}
		case TextureViewDimension::Texture1D_Array:
		{
			desc.Texture1DArray.MipSlice = range.base_mip_level;
			desc.Texture1DArray.FirstArraySlice = range.array_base;
			desc.Texture1DArray.ArraySize = range.array_count;


			break;
		}
		case TextureViewDimension::Texture2D:
		{
			desc.Texture2D.MipSlice = range.base_mip_level;
			desc.Texture2D.PlaneSlice = 0;

			break;
		}
		case TextureViewDimension::Texture2D_Array:
		{
			desc.Texture2DArray.FirstArraySlice = range.array_base;
			desc.Texture2DArray.ArraySize = range.array_count;

			desc.Texture2DArray.MipSlice = range.base_mip_level;
			desc.Texture2DArray.PlaneSlice = 0;


			break;
		}
		case TextureViewDimension::Texture3D:
		{
			desc.Texture3D.MipSlice = range.base_mip_level;
			desc.Texture3D.FirstWSlice = range.array_base;
			desc.Texture3D.WSize = range.array_count;

			break;
		}
		default:
			assert(false);
		}

		return desc;
	}

}