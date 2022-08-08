#pragma once
#include "../DX12CommonIncludes.h"
#include "../../RHITypes.h"

namespace mira
{
	D3D12_HEAP_TYPE to_internal(MemoryType type);
	D3D12_RESOURCE_FLAGS to_internal(UsageIntent usage);
	DXGI_FORMAT to_internal(ResourceFormat format);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC to_internal(GraphicsPipelineDesc desc, ID3D12RootSignature* rootsig);
	D3D12_SHADER_BYTECODE to_internal(const CompiledShader* compiled_shader);
	D3D12_RASTERIZER_DESC to_internal(const RasterizerStateDesc& desc);
	D3D12_DEPTH_STENCIL_DESC to_internal(const DepthStencilDesc& desc);
	D3D12_BLEND_DESC to_internal(const BlendDesc& desc);
	D3D12_PRIMITIVE_TOPOLOGY_TYPE to_internal(PrimitiveTopology topology);
	D3D_PRIMITIVE_TOPOLOGY to_internal_topology(PrimitiveTopology topology, u8 patch_control_points);
	D3D12_FILL_MODE to_internal(FillMode mode);
	D3D12_CULL_MODE to_internal(CullMode mode);
	D3D12_DEPTH_WRITE_MASK to_internal(DepthWriteMask mask);
	D3D12_COMPARISON_FUNC to_internal(ComparisonFunc func);
	D3D12_DEPTH_STENCILOP_DESC to_internal(const DepthStencilOpDesc& desc);
	D3D12_STENCIL_OP to_internal(StencilOp op);
	D3D12_RENDER_TARGET_BLEND_DESC to_internal(const RenderTargetBlendDesc& desc);
	D3D12_BLEND to_internal(BlendFactor fac);
	D3D12_BLEND_OP to_internal(BlendOp op);
	D3D12_LOGIC_OP to_internal(LogicOp op);
	u8 to_internal(ColorWriteEnable mask);
	D3D12_RESOURCE_DIMENSION to_internal(TextureType type);
	D3D12_RTV_DIMENSION to_internal_rtv(TextureType type);
	D3D12_DSV_DIMENSION to_internal_dsv(TextureType type);
	D3D12_SRV_DIMENSION to_internal_srv(TextureType type);
	D3D12_RENDER_PASS_RENDER_TARGET_DESC to_internal(const RenderPassTargetDesc& desc);
	D3D12_RENDER_PASS_DEPTH_STENCIL_DESC to_internal(const RenderPassDepthStencilTargetDesc& desc);
	D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE to_internal(RenderPassBeginAccessType access);
	D3D12_RENDER_PASS_ENDING_ACCESS_TYPE to_internal(RenderPassEndingAccessType access);
	D3D12_RENDER_PASS_FLAGS to_internal(RenderPassFlag flags);
}
