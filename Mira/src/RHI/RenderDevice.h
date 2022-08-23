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

		virtual void create_buffer(Buffer handle, const BufferDesc& desc) = 0;
		virtual void create_texture(Texture handle, const TextureDesc& desc) = 0;
		virtual void create_graphics_pipeline(Pipeline handle, const GraphicsPipelineDesc& desc) = 0;
		virtual void create_renderpass(RenderPass handle, const RenderPassDesc& desc) = 0;
		virtual void create_view(BufferView handle, Buffer buffer, const BufferViewDesc& desc) = 0;
		virtual void create_view(TextureView handle, Texture texture, const TextureViewDesc& desc) = 0;

		// Users determines when it is appropriate to free the resources
		virtual void free_buffer(Buffer handle) = 0;
		virtual void free_texture(Texture handle) = 0;
		virtual void free_pipeline(Pipeline handle) = 0;
		virtual void free_renderpass(RenderPass handle) = 0;
		virtual void free_view(BufferView handle) = 0;
		virtual void free_view(TextureView handle) = 0;
		virtual void recycle_sync(SyncReceipt receipt) = 0;
		virtual void recycle_command_list(RenderCommandList* list) = 0;

		// Grab GPU-accessible resource handle
		/*
			Future notes:
				If using multipe devices simultaneously, we somehow need to ensure that the global descriptors are identical.
				(This matters only if we are filling GPU buffer data manually --> Filling index indirection manually)

				Otherwise, we have to resolve render-device-specific global descriptors.
				
				Making sure that the global descriptors are identical across the simultaneously used render devices is likely easiest.
		*/	
		virtual u32 get_global_descriptor(BufferView view) const = 0;
		virtual u32 get_global_descriptor(TextureView view) const = 0;

		// CPU side wait, automatically recycles the receipt (recycles sync)
		virtual void wait_for_gpu(SyncReceipt receipt) = 0;
		virtual void flush() = 0;

		// Copies to a host-visible buffer
		virtual void upload_to_buffer(Buffer buffer, u32 dst_offset, void* data, u32 size) = 0;


		virtual RenderCommandList* allocate_command_list(QueueType queue = QueueType::Graphics) = 0;

		virtual void submit_command_lists(
			std::span<RenderCommandList*> lists,
			QueueType queue = QueueType::Graphics,
			std::optional<SyncReceipt> incoming_sync = std::nullopt,				// Synchronize with prior to command list execution
			std::optional<SyncReceipt> outgoing_sync = std::nullopt) = 0;			// Generate sync after command list execution


		virtual ~RenderDevice() {}

	};
}

