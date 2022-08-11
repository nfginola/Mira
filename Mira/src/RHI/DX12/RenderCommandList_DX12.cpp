#include "RenderCommandList_DX12.h"
#include "RenderDevice_DX12.h"
#include "Utilities/DX12RenderPass.h"

#include <iostream>

namespace mira
{
	RenderCommandList_DX12::RenderCommandList_DX12(
		const RenderDevice_DX12* device, 
		ComPtr<ID3D12CommandAllocator> allocator, 
		ComPtr<ID3D12GraphicsCommandList4> command_list,
		QueueType queue_type) :
		m_device(device),
		m_cmd_ator(allocator),
		m_cmd_list(command_list),
		m_queue_type(queue_type)
	{
	}
	
	void RenderCommandList_DX12::set_pipeline(Pipeline pipe)
	{
		m_cmd_list->SetPipelineState(m_device->get_api_pipeline(pipe));
		m_cmd_list->IASetPrimitiveTopology(m_device->get_api_topology(pipe));
	}

	void RenderCommandList_DX12::set_index_buffer(Buffer buffer)
	{
		auto res = m_device->get_api_buffer(buffer);
		D3D12_INDEX_BUFFER_VIEW ibv{};
		ibv.BufferLocation = res->GetGPUVirtualAddress();
		ibv.Format = DXGI_FORMAT_R32_UINT;
		ibv.SizeInBytes = m_device->get_api_buffer_size(buffer);
		m_cmd_list->IASetIndexBuffer(&ibv);
	}
	
	void RenderCommandList_DX12::draw(u32 verts_per_instance, u32 instance_count, u32 vert_start, u32 instance_start)
	{
		m_cmd_list->DrawInstanced(verts_per_instance, instance_count, vert_start, instance_start);
	}

	void RenderCommandList_DX12::update_shader_args(u8 num_descriptors, u32* descriptors, QueueType queue)
	{
		assert(queue != QueueType::Copy);

		if (queue == QueueType::Graphics)
		{
			for (u32 i = 0; i < num_descriptors; ++i)
			{
				m_cmd_list->SetGraphicsRoot32BitConstant(i, descriptors[i], 0);
			}
		}
		else if (queue == QueueType::Compute)
		{
			for (u32 i = 0; i < num_descriptors; ++i)
			{
				m_cmd_list->SetComputeRoot32BitConstant(i, descriptors[i], 0);
			}
		}
	}

	void RenderCommandList_DX12::begin_renderpass(RenderPass rp)
	{
		assert(!m_curr_rp.has_value());
		m_curr_rp = rp;
	
		// Bind render pass
		auto rts = m_device->get_rp_rts(rp);
		auto ds = m_device->get_rp_depth_stencil(rp);
		auto flags = m_device->get_rp_flags(rp);
		m_cmd_list->BeginRenderPass((u32)rts.size(), rts.data(), ds.has_value() ? &(*ds) : nullptr, flags);

		D3D12_RECT rect{};
		rect.left = 0;
		rect.right = 1600;
		rect.top = 0;
		rect.bottom = 900;
		m_cmd_list->RSSetScissorRects(1, &rect);

		D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.f, 0.f, 1600.f, 900.f, 0.f, D3D12_MAX_DEPTH);
		m_cmd_list->RSSetViewports(1, &vp);
	}

	void RenderCommandList_DX12::end_renderpass()
	{
		assert(m_curr_rp.has_value());

		// End render pass
		m_cmd_list->EndRenderPass();

		m_curr_rp = {};
	}

	void RenderCommandList_DX12::add_uav_barrier(Texture resource)
	{
	}

	void RenderCommandList_DX12::add_uav_barrier(Buffer resource)
	{
	}

	void RenderCommandList_DX12::add_aliasing_barrier(Texture before, Texture after)
	{
	}

	void RenderCommandList_DX12::add_transition_barrier(Buffer resource, ResourceState before, ResourceState after)
	{
		auto barr = CD3DX12_RESOURCE_BARRIER::Transition(
			m_device->get_api_buffer(resource),
			m_device->get_resource_state(before), m_device->get_resource_state(after),
			D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		m_cmd_list->ResourceBarrier(1, &barr);
	}

	void RenderCommandList_DX12::add_transition_barrier(Texture resource, u8 subresource, ResourceState before, ResourceState after)
	{
		auto barr = CD3DX12_RESOURCE_BARRIER::Transition(
			m_device->get_api_texture(resource),
			m_device->get_resource_state(before), m_device->get_resource_state(after),
			subresource);
		m_cmd_list->ResourceBarrier(1, &barr);
	}

	void RenderCommandList_DX12::flush_barriers()
	{
	}

	void RenderCommandList_DX12::submit_barriers(u8 num_barriers, ResourceBarrier* barriers)
	{
		std::vector<D3D12_RESOURCE_BARRIER> barrs{};
		barrs.reserve(num_barriers);

		for (u8 i = 0; i < num_barriers; ++i)
		{
			// translate barrier
			const auto& barr_in = barriers[i].barrier;

			switch (barr_in.type)
			{
			case ResourceBarrier::Type::Aliasing:
			{
				

				break;
			}
			case ResourceBarrier::Type::Transition:
			{
				break;
			}
			case ResourceBarrier::Type::UnorderedAccess:
			{
				break;
			}
			default:
				assert(false);
			}
		}

		m_cmd_list->ResourceBarrier(barrs.size(), barrs.data());
	}

	void RenderCommandList_DX12::open()
	{
		m_cmd_ator->Reset();
		m_cmd_list->Reset(m_cmd_ator.Get(), nullptr);

		// Set globals
		if (m_queue_type == QueueType::Graphics)
			m_cmd_list->SetGraphicsRootSignature(m_device->get_api_global_rsig());
		else if (m_queue_type == QueueType::Compute)
			m_cmd_list->SetComputeRootSignature(m_device->get_api_global_rsig());

		ID3D12DescriptorHeap* dheaps[] = { m_device->get_api_global_resource_dheap(), m_device->get_api_global_sampler_dheap() };
		m_cmd_list->SetDescriptorHeaps(2, dheaps);
	}

	void RenderCommandList_DX12::close()
	{
		m_cmd_list->Close();
	}
}

