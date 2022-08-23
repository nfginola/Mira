#pragma once
#include "../RenderDevice.h"
#include "DX12CommonIncludes.h"
#include "Utilities/DX12DescriptorChunk.h"
#include "Utilities/DX12Fence.h"

#include <unordered_map>
#include <queue>
#include <functional>
#include <optional>

namespace D3D12MA { class Allocator; class Allocation; }
class DX12DescriptorManager;
class DX12Queue;

namespace mira
{
	class RenderCommandList_DX12;
	class SwapChain_DX12;

	class RenderDevice_DX12 final : public RenderDevice
	{
	public:
		RenderDevice_DX12(ComPtr<ID3D12Device5> device, IDXGIAdapter* adapter, bool debug);
		~RenderDevice_DX12();

		// Pass buffer handles to attach swapchain buffers to.
		// Number of passed handles is the number of buffers used for the swapchain!
		SwapChain* create_swapchain(void* hwnd, std::span<Texture> swapchain_buffer_handles);

		// Resource creation/destruction
		void create_buffer(const BufferDesc& desc, Buffer handle);
		void create_texture(const TextureDesc& desc, Texture handle);
		void create_graphics_pipeline(const GraphicsPipelineDesc& desc, Pipeline handle);
		void create_renderpass(const RenderPassDesc& desc, RenderPass handle);

		void create_view(Buffer buffer, const BufferViewDesc& desc, BufferView handle);
		void create_view(Texture texture, const TextureViewDesc& desc, TextureView handle);
		

		// Sensitive resources that may be in-flight
		void free_buffer(Buffer handle);
		void free_texture(Texture handle);
		void free_pipeline(Pipeline handle);
		void free_renderpass(RenderPass handle);
		void free_view(BufferView handle);
		void free_view(TextureView handle);


		void recycle_sync(SyncReceipt receipt);
		void recycle_command_list(RenderCommandList* list);

		void upload_to_buffer(Buffer buffer, u32 dst_offset, void* data, u32 size);

		void flush();
		void wait_for_gpu(SyncReceipt receipt);
		
		// Grab a GPU accessible view to a specific resource
		u32 get_global_descriptor(Buffer buffer, u32 view) const;
		u32 get_global_descriptor(Texture texture, u32 view) const;

		RenderCommandList* allocate_command_list(QueueType queue = QueueType::Graphics);

		void submit_command_lists(
			std::span<RenderCommandList*> lists,
			QueueType queue = QueueType::Graphics,
			std::optional<SyncReceipt> incoming_sync = std::nullopt,
			std::optional<SyncReceipt> outgoing_sync = std::nullopt);



		// Implementation interfaces
		ID3D12Resource* get_api_buffer(Buffer buffer) const;
		ID3D12Resource* get_api_texture(Texture texture) const;
		u32 get_api_buffer_size(Buffer buffer) const;

		D3D12_RESOURCE_STATES get_resource_state(ResourceState state) const;

		D3D_PRIMITIVE_TOPOLOGY get_api_topology(Pipeline pipeline) const;
		ID3D12PipelineState* get_api_pipeline(Pipeline pipeline) const;
		ID3D12RootSignature* get_api_global_rsig() const;
		ID3D12DescriptorHeap* get_api_global_resource_dheap() const;
		ID3D12DescriptorHeap* get_api_global_sampler_dheap() const;

		const std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC>& get_rp_rts(RenderPass rp) const;
		std::optional<D3D12_RENDER_PASS_DEPTH_STENCIL_DESC> get_rp_depth_stencil(RenderPass rp) const;
		D3D12_RENDER_PASS_FLAGS get_rp_flags(RenderPass rp) const;

		ID3D12CommandQueue* get_queue(D3D12_COMMAND_LIST_TYPE type);

		void register_swapchain_texture(ComPtr<ID3D12Resource> texture, Texture handle);
		void set_clear_color(Texture tex, const std::array<float, 4>& clear_color);

			
	private:
		void create_queues();
		void init_dma(IDXGIAdapter* adapter);
		void init_rootsig();
		std::vector<D3D12_STATIC_SAMPLER_DESC> grab_static_samplers();
		DX12Queue* get_queue(QueueType type);


	private:
		struct TextureView_Storage
		{
			Texture tex;
			ViewType type{ ViewType::None };
			DX12DescriptorChunk view;

			TextureView_Storage(Texture tex_in, ViewType type_in, const DX12DescriptorChunk& descriptor) : tex(tex_in), type(type_in), view(descriptor) {}
		};

		struct BufferView_Storage
		{
			Buffer buf;
			ViewType type{ ViewType::None };
			DX12DescriptorChunk view;

			BufferView_Storage(Buffer buf_in, ViewType type_in, const DX12DescriptorChunk& descriptor) : buf(buf_in), type(type_in), view(descriptor) {}
		};

		struct GPUResource_Storage
		{
			ComPtr<D3D12MA::Allocation> alloc;
			ComPtr<ID3D12Resource> resource;
		};

		struct Buffer_Storage : public GPUResource_Storage
		{
			BufferDesc desc;
			u8* mapped_resource{ nullptr };
		};

		struct Texture_Storage : public GPUResource_Storage
		{
			TextureDesc desc;
		};

		struct Pipeline_Storage
		{
			GraphicsPipelineDesc desc;
			D3D_PRIMITIVE_TOPOLOGY topology{};
			ComPtr<ID3D12PipelineState> pipeline;
		};

		struct RenderPass_Storage
		{
			RenderPassDesc desc;
			std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> render_targets;
			std::optional<D3D12_RENDER_PASS_DEPTH_STENCIL_DESC> depth_stencil{};
			D3D12_RENDER_PASS_FLAGS flags{};
		};

		struct SyncPrimitive
		{
			DX12Fence fence;
		};

	private:
		ComPtr<ID3D12Device5> m_device;
		bool m_debug_on{ false };
		std::unique_ptr<DX12Queue> m_direct_queue, m_copy_queue, m_compute_queue;
		ComPtr<D3D12MA::Allocator> m_dma;

		ComPtr<ID3D12RootSignature> m_common_rsig;
		std::unique_ptr<DX12DescriptorManager> m_descriptor_mgr;

		/*
			Resources
		*/
		std::vector<std::optional<Buffer_Storage>> m_buffers;
		std::vector<std::optional<Texture_Storage>> m_textures;
		std::vector<std::optional<BufferView_Storage>> m_buffer_views;
		std::vector<std::optional<TextureView_Storage>> m_texture_views;
		std::vector<std::optional<Pipeline_Storage>> m_pipelines;
		std::vector<std::optional<RenderPass_Storage>> m_renderpasses;

		std::vector<std::optional<SyncPrimitive>> m_syncs;		
		std::queue<SyncPrimitive> m_recycled_syncs;


		// Important that this is destructed before resources and descriptor managers (need to free underlying texture)
		std::unique_ptr<SwapChain_DX12> m_swapchain;


		// Reuse existing command allocators
		std::queue<std::unique_ptr<RenderCommandList_DX12>> m_cmd_list_pool_direct, m_cmd_list_pool_compute, m_cmd_list_pool_copy;
		std::unordered_map<RenderCommandList_DX12*, std::unique_ptr<RenderCommandList_DX12>> m_cmd_lists_in_play;




	};
		
	// Helper for inserting/getting resources in an array-like manner
	template <typename T>
	void try_insert(std::vector<std::optional<T>>& vec, const T& element, u32 index)
	{
		// resize if needed
		if (vec.size() <= index)
			vec.resize(vec.size() * 4);

		assert(!vec[index].has_value());
		vec[index] = element;
	}

	template <typename T>
	const T& try_get(const std::vector<std::optional<T>>& vec, u32 index)
	{
		assert(vec.size() > index);
		assert(vec[index].has_value());
		return *(vec[index]);
	}

	template <typename T>
	T& try_get(std::vector<std::optional<T>>& vec, u32 index)
	{
		assert(vec.size() > index);
		assert(vec[index].has_value());
		return *(vec[index]);
	}
}

