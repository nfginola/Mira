#pragma once
#include "RHITypes.h"
#include "RenderCommandList.h"
#include "SwapChain.h"

namespace mira
{
	class RenderDevice
	{
	public:

		virtual SwapChain* create_swapchain(void* hwnd, u8 num_buffers) = 0;

		virtual Buffer create_buffer(const BufferDesc& desc) = 0;
		virtual Texture create_texture(const TextureDesc& desc) = 0;
		virtual Pipeline create_graphics_pipeline(const GraphicsPipelineDesc& desc) = 0;
		virtual RenderPass create_renderpass(const RenderPassDesc& desc) = 0;
		virtual BufferView create_view(Buffer buffer, const BufferViewDesc& desc) = 0;
		virtual TextureView create_view(Texture texture, const TextureViewDesc& desc) = 0;

		// Users determines when it is appropriate to free the resources (they may be in flight!)
		virtual void free_buffer(Buffer handle) = 0;
		virtual void free_texture(Texture handle) = 0;
		virtual void free_pipeline(Pipeline handle) = 0;
		virtual void free_renderpass(RenderPass handle) = 0;
		virtual void free_view(BufferView handle) = 0;
		virtual void free_view(TextureView handle) = 0;
		virtual void recycle_sync(SyncReceipt receipt) = 0;
		virtual void recycle_command_list(CommandList handle) = 0;

		/*
			Motivation for why allocation and compilation of a command list is exposed:
				The expected use case is that the application (on a single thread) allocates multiple metadata for recording multiple lists (a single metadata likely maps to allocator + list pair).
				This means that all the data to be processed has been grabbed from various shared data structures and can be dealt with in parallel.

				For example, you have a list of { CommandListHandle, RenderCommandList }, the calling user can appropriately ration
				a chunk of such pairs from the list to various threads for compilation (e.g a Job System).

				Each compilation only touches data allocated and assigned to CommandListHandle.
		*/

		// Reserve metadata for command recording
		virtual CommandList allocate_command_list(QueueType queue = QueueType::Graphics) = 0;

		// Compile backend representation of the command list
		virtual void compile_command_list(CommandList handle, RenderCommandList list) = 0;

		virtual std::optional<SyncReceipt> submit_command_lists(
			std::span<CommandList> lists,
			QueueType queue = QueueType::Graphics,
			std::optional<SyncReceipt> incoming_sync = std::nullopt,				// Synchronize with prior to command list execution
			bool generate_sync = false) = 0;												// Generate sync after command list execution


		// Grab GPU-accessible resource handle
		virtual u32 get_global_descriptor(BufferView view) const = 0;
		virtual u32 get_global_descriptor(TextureView view) const = 0;

		// CPU side wait, automatically recycles the receipt (recycles sync)
		virtual void wait_for_gpu(SyncReceipt receipt) = 0;

		virtual void flush() = 0;

		virtual u8* map(Buffer handle, u32 subresource = 0, std::pair<u32, u32> read_range = { 0, 0 }) = 0;
		virtual void unmap(Buffer handle, u32 subresource = 0, std::pair<u32, u32> written_range = { 0, 0 }) = 0;

		// Copies to a host-visible buffer
		virtual void upload_to_buffer(Buffer buffer, u32 dst_offset, void* data, u32 size) = 0;









		virtual ~RenderDevice() {}

	};
}

