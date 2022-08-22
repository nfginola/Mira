#pragma once
#include "RHITypes.h"
#include "RenderCommandList.h"
#include "SwapChain.h"

namespace mira
{
	class RenderDevice
	{
	public:
		virtual SwapChain* create_swapchain(void* hwnd, std::span<Texture> swapchain_buffer_handles) = 0;

		virtual void create_buffer(const BufferDesc& desc, Buffer handle) = 0;
		virtual void create_texture(const TextureDesc& desc, Texture handle) = 0;
		virtual void create_graphics_pipeline(const GraphicsPipelineDesc& desc, Pipeline handle) = 0;
		virtual void create_renderpass(const RenderPassDesc& desc, RenderPass handle) = 0;

		// Users determines when it is appropriate to free the resources
		virtual void free_buffer(Buffer handle) = 0;
		virtual void free_texture(Texture handle) = 0;
		virtual void free_pipeline(Pipeline handle) = 0;
		virtual void free_renderpass(RenderPass handle) = 0;
		virtual void recycle_sync(SyncReceipt receipt) = 0;
		virtual void recycle_command_list(RenderCommandList* list) = 0;

		// CPU side wait, automatically recycles the receipt (recycles sync)
		virtual void wait_for_gpu(SyncReceipt receipt) = 0;
		virtual void flush() = 0;

		// Copies to a host-visible buffer
		virtual void upload_to_buffer(Buffer buffer, u32 dst_offset, void* data, u32 size) = 0;

		// Replace this with a mira::ResourceView that is passed like all others.
		// no internally generated handles
		// keep it global 
		virtual u32 create_view(Buffer buffer, ViewType view, u32 offset, u32 stride, u32 count = 1, bool raw = false) = 0;
		
		// Grab GPU-accessible resource handle
		virtual u32 get_global_descriptor(Buffer buffer, u32 view) const = 0;
		virtual u32 get_global_descriptor(Texture texture, u32 view) const = 0;

		virtual RenderCommandList* allocate_command_list(QueueType queue = QueueType::Graphics) = 0;

		/*
			
			Refactor SyncReceipt too so that the handle is application owned.

			instead of generate_sync --> Pass in another std::optional<SyncReceipt>
		
		*/



		// Submit command lists with the option to consume and generate a sync receipt for incoming and outgoing synchronization
		virtual void submit_command_lists(
			std::span<RenderCommandList*> lists,
			QueueType queue = QueueType::Graphics,
			std::optional<SyncReceipt> incoming_sync = std::nullopt,				// Synchronize with prior to command list execution
			std::optional<SyncReceipt> outgoing_sync = std::nullopt) = 0;			// Generate sync after command list execution


		virtual ~RenderDevice() {}

	};
}

