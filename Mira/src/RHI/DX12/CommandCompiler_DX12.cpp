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
		// Set globals
		if (m_queue_type == QueueType::Graphics)
			m_list->SetGraphicsRootSignature(m_dev->get_api_global_rsig());
		else if (m_queue_type == QueueType::Compute)
			m_list->SetComputeRootSignature(m_dev->get_api_global_rsig());

		ID3D12DescriptorHeap* dheaps[] = { m_dev->get_api_global_resource_dheap(), m_dev->get_api_global_sampler_dheap() };
		m_list->SetDescriptorHeaps(2, dheaps);
	}


	void CommandCompiler_DX12::compile(const RenderCommandDraw& cmd)
	{
		m_list->DrawInstanced(cmd.verts_per_instance, cmd.instance_count, cmd.vert_start, cmd.instance_start);
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
}
