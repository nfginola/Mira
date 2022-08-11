#pragma once
#include "DX12CommonIncludes.h"
#include "../RenderCommandList.h"

namespace mira
{
	class RenderDevice_DX12;

	class RenderCommandList_DX12 final : public RenderCommandList
	{
	public:
		RenderCommandList_DX12(
			const RenderDevice_DX12* device, 
			ComPtr<ID3D12CommandAllocator> allocator, 
			ComPtr<ID3D12GraphicsCommandList4> command_list,
			QueueType queue_type);
		
		void set_pipeline(Pipeline pipe);
		void set_index_buffer(Buffer buffer);
		void draw(u32 verts_per_instance, u32 instance_count, u32 vert_start, u32 instance_start);
		void update_shader_args(u8 num_descriptors, u32* descriptors, QueueType queue);
		void begin_renderpass(RenderPass rp);
		void end_renderpass();

		void add_uav_barrier(Texture resource);
		void add_uav_barrier(Buffer resource);
		void add_aliasing_barrier(Texture before, Texture after);
		void add_transition_barrier(Buffer resource, ResourceState before, ResourceState after);
		void add_transition_barrier(Texture resource, u8 subresource, ResourceState before, ResourceState after);
		void flush_barriers();

		void submit_barriers(u8 num_barriers, ResourceBarrier* barriers);



		// Implementation interface
		auto get_allocator_and_list() const { return std::pair{ m_cmd_ator.Get(), m_cmd_list.Get() }; }
		void open();
		void close();
		QueueType queue_type() const { return m_queue_type; }

	private:
		// Provide non-modifying access to any component from the Render Device
		const RenderDevice_DX12* m_device{ nullptr };

		ComPtr<ID3D12CommandAllocator> m_cmd_ator;
		ComPtr<ID3D12GraphicsCommandList4> m_cmd_list;

		QueueType m_queue_type{ QueueType::None };

		// State
		std::optional<RenderPass> m_curr_rp;
	};
}


