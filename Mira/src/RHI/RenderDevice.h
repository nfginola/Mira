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

		// User determines when it is appropriate for the system to re-use the resources associated with this sync receipt
		virtual void recycle_sync(SyncReceipt receipt) = 0;
		virtual void recycle_command_list(RenderCommandList* list) = 0;

		// CPU side wait, automatically consumes the receipt (recycles sync)
		virtual void wait_for_gpu(SyncReceipt&& receipt) = 0;
		virtual void flush() = 0;

		// Copies to a mapped buffer
		virtual void upload_to_buffer(Buffer buffer, u32 dst_offset, void* data, u32 size) = 0;

		// We won't allow view dropping, until we see a need for it.
		// Views for a resource are dropped automatically when that resource is freed.
		virtual u32 create_view(Buffer buffer, ViewType view, u32 offset, u32 stride, u32 count = 1, bool raw = false) = 0;


		virtual u32 get_global_descriptor(Buffer buffer, u32 view) const = 0;
		virtual u32 get_global_descriptor(Texture texture, u32 view) const = 0;

		virtual RenderCommandList* allocate_command_list(QueueType queue = QueueType::Graphics) = 0;

		// Submit command lists with the option to consume and generate a sync receipt for incoming and outgoing synchronization
		virtual std::optional<SyncReceipt> submit_command_lists(
			std::span<RenderCommandList*> lists,
			QueueType queue = QueueType::Graphics,
			bool generate_sync = false, std::optional<SyncReceipt> sync_with = std::nullopt) = 0;


		virtual ~RenderDevice() {}

	};
}

