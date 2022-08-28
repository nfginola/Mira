#include "CommandCompiler_DX12.h"
#include "RenderDevice_DX12.h"

namespace mira
{
	CommandCompiler_DX12::CommandCompiler_DX12(const RenderDevice_DX12* dev, ComPtr<ID3D12CommandAllocator> ator, ComPtr<ID3D12GraphicsCommandList4> cmdl, QueueType queue) :
		m_dev(dev),
		m_ator(ator),
		m_list(cmdl),
		m_queue_type(queue)
	{
		/*
			Ordering constraint between SetDescriptorHeap and SetRootSig
			https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_DynamicResources.html
			" SetDescriptorHeaps must be called, passing the corresponding heaps, before a call to SetGraphicsRootSignature or SetComputeRootSignature "
		*/
		if (m_queue_type == QueueType::Graphics || m_queue_type == QueueType::Compute)
		{
			ID3D12DescriptorHeap* dheaps[] = { m_dev->get_api_global_resource_dheap() };
			m_list->SetDescriptorHeaps(_countof(dheaps), dheaps);
		}


		// Set globals
		if (m_queue_type == QueueType::Graphics)
			m_list->SetGraphicsRootSignature(m_dev->get_api_global_rsig());
		else if (m_queue_type == QueueType::Compute)
			m_list->SetComputeRootSignature(m_dev->get_api_global_rsig());

	}


	void CommandCompiler_DX12::compile(const RenderCommandDraw& cmd)
	{
		m_list->DrawInstanced(cmd.verts_per_instance, cmd.instance_count, cmd.vert_start, cmd.instance_start);
	}

	void CommandCompiler_DX12::compile(const RenderCommandDrawIndexed& cmd)
	{
		if (cmd.index_buffer.handle != m_current_ib.handle)
		{
			auto ib = m_dev->get_api_buffer(cmd.index_buffer);
			D3D12_INDEX_BUFFER_VIEW ibv{};
			ibv.BufferLocation = ib->GetGPUVirtualAddress();
			ibv.Format = DXGI_FORMAT_R32_UINT;
			ibv.SizeInBytes = m_dev->get_api_buffer_size(cmd.index_buffer);
			m_list->IASetIndexBuffer(&ibv);

			m_current_ib = cmd.index_buffer;
		}

		m_list->DrawIndexedInstanced(cmd.indices_per_instance, cmd.instance_count, cmd.index_start, cmd.vertex_start, cmd.instance_start);
	}

	void CommandCompiler_DX12::compile(const RenderCommandSetPipeline& cmd)
	{
		m_list->SetPipelineState(m_dev->get_api_pipeline(cmd.pipeline));
		m_list->IASetPrimitiveTopology(m_dev->get_api_topology(cmd.pipeline));

	}

	void CommandCompiler_DX12::compile(const RenderCommandBeginRenderPass& cmd)
	{
		// Bind render pass
		auto rts = m_dev->get_rp_rts(cmd.rp);
		auto ds = m_dev->get_rp_depth_stencil(cmd.rp);
		auto flags = m_dev->get_rp_flags(cmd.rp);
		m_list->BeginRenderPass((u32)rts.size(), rts.data(), ds.has_value() ? &(*ds) : nullptr, flags);

		/*
			These should be moved to another command. I dont know where though..
		*/

		D3D12_RECT rect{};
		rect.left = 0;
		rect.right = 1600;
		rect.top = 0;
		rect.bottom = 900;
		m_list->RSSetScissorRects(1, &rect);

		D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.f, 0.f, 1600.f, 900.f, 0.f, D3D12_MAX_DEPTH);
		m_list->RSSetViewports(1, &vp);
	}

	void CommandCompiler_DX12::compile(const RenderCommandEndRenderPass& cmd)
	{
		m_list->EndRenderPass();
	}

	void CommandCompiler_DX12::compile(const RenderCommandBarrier& cmd)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barrs{};
		barrs.reserve(cmd.barriers.size());

		for (const auto& barr : cmd.barriers)
		{
			switch (barr.info.type)
			{
			case ResourceBarrier::Type::Aliasing:
			{
				assert(false);
				break;
			}
			case ResourceBarrier::Type::Transition:
			{
				if (barr.info.res_type == ResourceBarrier::ResourceType::Buffer)
				{
					barrs.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
						m_dev->get_api_buffer(Buffer{ barr.info.resource_or_before }),
						m_dev->get_resource_state(barr.info.state_before), m_dev->get_resource_state(barr.info.state_after),
						barr.info.subresource));
				}
				else
				{
					barrs.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
						m_dev->get_api_texture(Texture{ barr.info.resource_or_before }),
						m_dev->get_resource_state(barr.info.state_before), m_dev->get_resource_state(barr.info.state_after),
						barr.info.subresource));
				}

				break;
			}
			case ResourceBarrier::Type::UnorderedAccess:
			{
				assert(false);
				break;
			}
			default:
				assert(false);
			}
		}

		m_barriers_per_submission.push_back(std::move(barrs));
		m_list->ResourceBarrier((u32)m_barriers_per_submission.back().size(), m_barriers_per_submission.back().data());
	}

	void CommandCompiler_DX12::compile(const RenderCommandCopyBuffer& cmd)
	{
		auto src = m_dev->get_api_buffer(cmd.src);
		auto dst = m_dev->get_api_buffer(cmd.dst);

		m_list->CopyBufferRegion(dst, cmd.dst_offset, src, cmd.src_offset, cmd.size);
	}

	void CommandCompiler_DX12::compile(const RenderCommandCopyBufferToImage& cmd)
	{
		D3D12_TEXTURE_COPY_LOCATION dst_loc{}, src_loc{};

		dst_loc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst_loc.pResource = m_dev->get_api_texture(cmd.dst);
		dst_loc.SubresourceIndex = cmd.dst_subresource;

		src_loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src_loc.pResource = m_dev->get_api_buffer(cmd.src);
		src_loc.PlacedFootprint.Offset = cmd.src_offset;				// ======= @todo: is this correct?

		// Assert that the user has placed the data correctly according to alignment rules
		assert(cmd.src_offset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT == 0);

		src_loc.PlacedFootprint.Footprint.RowPitch = cmd.src_rowpitch;
		src_loc.PlacedFootprint.Footprint.Depth = cmd.src_depth;
		src_loc.PlacedFootprint.Footprint.Width = cmd.src_width;
		src_loc.PlacedFootprint.Footprint.Height = cmd.src_height;
		src_loc.PlacedFootprint.Footprint.Format = m_dev->get_format(cmd.src_format);

		m_list->CopyTextureRegion(&dst_loc, std::get<0>(cmd.dst_topleft), std::get<1>(cmd.dst_topleft), std::get<2>(cmd.dst_topleft), &src_loc, nullptr);
	}

	void CommandCompiler_DX12::compile(const RenderCommandUpdateShaderArgs& cmd)
	{
		if (m_queue_type == QueueType::Graphics)
			m_list->SetGraphicsRoot32BitConstants(0, cmd.num_constants, cmd.constants.data(), 0);
		else if (m_queue_type == QueueType::Compute)
			m_list->SetComputeRoot32BitConstants(0, cmd.num_constants, cmd.constants.data(), 0);

	}
}
