#include "RenderDevice_DX12.h"
#include <D3D12MemAlloc.h>
#include <iostream>

#include "Utilities/DX12DescriptorManager.h"
#include "Utilities/DX12Queue.h"
#include "Utilities/DX12Fence.h"
#include "Utilities/StructTranslator_DX12.h"
#include "RenderCommandList_DX12.h"

#include "SwapChain_DX12.h"


namespace mira
{

	RenderDevice_DX12::RenderDevice_DX12(ComPtr<ID3D12Device5> device, IDXGIAdapter* adapter, bool debug) :
		m_device(device),
		m_debug_on(debug)
	{
		// 0 marked as un-used
		//m_resources.resize(1);
		m_buffers.resize(1);
		m_textures.resize(1);
		m_pipelines.resize(1);
		m_renderpasses.resize(1);
		m_syncs.resize(1);

		m_buffer_views.resize(1);
		m_texture_views.resize(1);

		create_queues();
		init_dma(adapter);

		m_descriptor_mgr = std::make_unique<DX12DescriptorManager>(m_device.Get());

		init_rootsig();
	}

	RenderDevice_DX12::~RenderDevice_DX12()
	{
		// Destroy any leftover views automatically
		for (auto view : m_buffer_views)
		{
			if (view.has_value())
			{
				m_descriptor_mgr->free(&(*view).view);
			}
		}

		for (auto view : m_texture_views)
		{
			if (view.has_value())
			{
				m_descriptor_mgr->free(&(*view).view);
			}
		}
	}

	SwapChain* RenderDevice_DX12::create_swapchain(void* hwnd, std::span<Texture> swapchain_buffer_handles)
	{
		m_swapchain = std::make_unique<SwapChain_DX12>(this, (HWND)hwnd, swapchain_buffer_handles, m_debug_on);
		return m_swapchain.get();
	}

	void RenderDevice_DX12::create_buffer(const BufferDesc& desc, Buffer handle)
	{
		HRESULT hr{ S_OK };

		D3D12MA::ALLOCATION_DESC ad{};
		ad.HeapType = to_internal(desc.memory_type);

		D3D12_RESOURCE_DESC rd{};
		rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rd.Alignment = desc.alignment;
		rd.Width = desc.size;
		rd.Height = rd.DepthOrArraySize = rd.MipLevels = 1;
		rd.Format = DXGI_FORMAT_UNKNOWN;
		rd.SampleDesc.Count = 1;
		rd.SampleDesc.Quality = 0;
		rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rd.Flags = to_internal(desc.usage);

		D3D12_RESOURCE_STATES init_state{ D3D12_RESOURCE_STATE_COMMON };
		if (ad.HeapType == D3D12_HEAP_TYPE_UPLOAD)
			init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
		else if (ad.HeapType == D3D12_HEAP_TYPE_READBACK)
			init_state = D3D12_RESOURCE_STATE_COPY_DEST;

		Buffer_Storage storage{};
		storage.desc = desc;

		hr = m_dma->CreateResource(&ad, &rd, init_state, nullptr, storage.alloc.GetAddressOf(), IID_PPV_ARGS(storage.resource.GetAddressOf()));
		HR_VFY(hr);

		// Map memory if on upload heap
		if (desc.memory_type == MemoryType::Upload)
			storage.resource->Map(0, {}, (void**)&storage.mapped_resource);

		try_insert(m_buffers, storage, get_slot(handle.handle));
	}

	void RenderDevice_DX12::create_texture(const TextureDesc& desc, Texture handle)
	{
		HRESULT hr{ S_OK };

		D3D12MA::ALLOCATION_DESC ad{};
		ad.HeapType = to_internal(desc.memory_type);

		D3D12_RESOURCE_DESC rd{};
		rd.Dimension = to_internal(desc.type);
		rd.Alignment = desc.alignment;
		rd.Width = desc.width;
		rd.Height = desc.height;
		rd.DepthOrArraySize = desc.depth;
		rd.MipLevels = desc.mip_levels;
		rd.Format = to_internal(desc.format);
		rd.SampleDesc.Count = desc.sample_count;
		rd.SampleDesc.Quality = desc.sample_quality;
		rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rd.Flags = to_internal(desc.usage);

		D3D12_RESOURCE_STATES init_state{ D3D12_RESOURCE_STATE_COMMON };
		if (ad.HeapType == D3D12_HEAP_TYPE_UPLOAD)
			init_state = D3D12_RESOURCE_STATE_GENERIC_READ;
		else if (ad.HeapType == D3D12_HEAP_TYPE_READBACK)
			init_state = D3D12_RESOURCE_STATE_COPY_DEST;

		Texture_Storage storage{};
		storage.desc = desc;

		hr = m_dma->CreateResource(&ad, &rd, init_state, nullptr, storage.alloc.GetAddressOf(), IID_PPV_ARGS(storage.resource.GetAddressOf()));
		HR_VFY(hr);

		try_insert(m_textures, storage, get_slot(handle.handle));
	}

	void RenderDevice_DX12::create_graphics_pipeline(const GraphicsPipelineDesc& desc, Pipeline handle)
	{
		assert(!desc.vs->blob.empty() && !desc.ps->blob.empty());
		HRESULT hr{ S_OK };
			
		D3D12_GRAPHICS_PIPELINE_STATE_DESC api_desc = to_internal(desc, m_common_rsig.Get());

		ComPtr<ID3D12PipelineState> pso;
		hr = m_device->CreateGraphicsPipelineState(&api_desc, IID_PPV_ARGS(pso.GetAddressOf()));
		HR_VFY(hr);

		Pipeline_Storage storage{};
		storage.desc = desc;
		storage.topology = to_internal_topology(desc.topology, desc.num_control_patches);
		storage.pipeline = pso;
		try_insert(m_pipelines, storage, get_slot(handle.handle));
	}

	void RenderDevice_DX12::create_renderpass(const RenderPassDesc& desc, RenderPass handle)
	{
		//auto storage = std::make_shared<RenderPass_Storage>();
		//storage->desc = desc;
		//storage->flags = to_internal(desc.flags);
		RenderPass_Storage storage{};
		storage.desc = desc;
		storage.flags = to_internal(desc.flags);
		
		// Translate description to D3D12
		auto& descs = storage.render_targets;
		for (const auto& rtd : desc.render_target_descs)
		{
			auto& res = try_get(m_texture_views, get_slot(rtd.view.handle));		// grab view md

			auto api = to_internal(rtd);
			assert(res.type == ViewType::RenderTarget);
			api.cpuDescriptor = res.view.cpu_handle(0);

			auto& tex = try_get(m_textures, get_slot(res.tex.handle));				// grab underlying texture md
			api.BeginningAccess.Clear.ClearValue.Format = to_internal(tex.desc.format);
			api.BeginningAccess.Clear.ClearValue.Color[0] = tex.desc.clear_color[0];
			api.BeginningAccess.Clear.ClearValue.Color[1] = tex.desc.clear_color[1];
			api.BeginningAccess.Clear.ClearValue.Color[2] = tex.desc.clear_color[2];
			api.BeginningAccess.Clear.ClearValue.Color[3] = tex.desc.clear_color[3];

			// not supporting resolves for now
			descs.push_back(api);
		}

		// Translate depth stencil 
		if (desc.depth_stencil_desc.has_value())
		{
			auto& res = try_get(m_texture_views, get_slot(desc.depth_stencil_desc->view.handle));
			auto depth_api = to_internal(*desc.depth_stencil_desc);
			assert(res.type == ViewType::DepthStencil);
			depth_api.cpuDescriptor = res.view.cpu_handle(0);
			storage.depth_stencil = depth_api;
		}

		try_insert(m_renderpasses, storage, get_slot(handle.handle));
	}

	void RenderDevice_DX12::free_buffer(Buffer handle)
	{
		auto& res = try_get(m_buffers, get_slot(handle.handle));
		
		m_buffers[get_slot(handle.handle)] = std::nullopt;
	}

	void RenderDevice_DX12::free_texture(Texture handle)
	{
		auto& res = try_get(m_textures, get_slot(handle.handle));

		m_textures[get_slot(handle.handle)] = std::nullopt;
	}

	void RenderDevice_DX12::free_pipeline(Pipeline handle)
	{
		m_pipelines[get_slot(handle.handle)] = std::nullopt;
	}

	void RenderDevice_DX12::free_renderpass(RenderPass handle)
	{
		m_renderpasses[get_slot(handle.handle)] = std::nullopt;
	}

	void RenderDevice_DX12::free_view(BufferView handle)
	{
		auto& res = try_get(m_buffer_views, get_slot(handle.handle));
		m_descriptor_mgr->free(&res.view);

		m_buffer_views[get_slot(handle.handle)] = std::nullopt;
	}

	void RenderDevice_DX12::free_view(TextureView handle)
	{
		auto& res = try_get(m_texture_views, get_slot(handle.handle));
		m_descriptor_mgr->free(&res.view);

		m_texture_views[get_slot(handle.handle)] = std::nullopt;
	}
	void RenderDevice_DX12::create_view(Buffer buffer, const BufferViewDesc& desc, BufferView handle)
	{
		assert(desc.view != ViewType::None);
		assert(desc.view != ViewType::DepthStencil);
		assert(desc.view != ViewType::RenderTarget);

		auto& buffer_storage = try_get(m_buffers, get_slot(buffer.handle));
		auto view_desc = m_descriptor_mgr->allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		assert(desc.offset + desc.stride * desc.count <= buffer_storage.desc.size);

		if (desc.view == ViewType::Constant)
		{
			assert(desc.stride % 256 == 0);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvd{};
			cbvd.BufferLocation = buffer_storage.resource.Get()->GetGPUVirtualAddress();
			cbvd.SizeInBytes = desc.stride * desc.count;
			m_device->CreateConstantBufferView(&cbvd, view_desc.cpu_handle(0));
		}
		else if (desc.view == ViewType::ShaderResource)
		{
			assert(desc.offset % desc.stride == 0);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvd{};
			srvd.Format = DXGI_FORMAT_UNKNOWN;
			srvd.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvd.Buffer.FirstElement = desc.offset / desc.stride;
			srvd.Buffer.NumElements = desc.count;
			srvd.Buffer.StructureByteStride = desc.stride;
			srvd.Buffer.Flags = desc.raw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;

			m_device->CreateShaderResourceView(buffer_storage.resource.Get(), &srvd, view_desc.cpu_handle(0));

		}
		else if (desc.view == ViewType::UnorderedAccess)
		{
			assert(desc.offset % desc.stride == 0);

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavd{};
			uavd.Format = DXGI_FORMAT_UNKNOWN;
			uavd.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavd.Buffer.FirstElement = desc.offset / desc.stride;
			uavd.Buffer.NumElements = desc.count;
			uavd.Buffer.StructureByteStride = desc.stride;
			uavd.Buffer.CounterOffsetInBytes = 0;
			uavd.Buffer.Flags = desc.raw ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;

			// We never use counter buffers, user has to create their own RW buffer with counters and do InterlockedAdd
			// This makes counting explicit on the user side and simplifies API (always no counter)
			m_device->CreateUnorderedAccessView(buffer_storage.resource.Get(), nullptr, &uavd, view_desc.cpu_handle(2));
		}
		else if (desc.view == ViewType::RaytracingAS)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvd{};
			srvd.Format = DXGI_FORMAT_UNKNOWN;
			srvd.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvd.RaytracingAccelerationStructure.Location = desc.offset;

			m_device->CreateShaderResourceView(buffer_storage.resource.Get(), &srvd, view_desc.cpu_handle(0));
		}
		else
		{
			assert(false);
		}

		try_insert(m_buffer_views, BufferView_Storage(buffer, desc.view, view_desc), get_slot(handle.handle));

	}

	void RenderDevice_DX12::create_view(Texture texture, const TextureViewDesc& desc, TextureView handle)
	{
		assert(desc.view != ViewType::None);
		assert(desc.view != ViewType::Constant);
		assert(desc.view != ViewType::RaytracingAS);

		auto& tex_storage = try_get(m_textures, get_slot(texture.handle));

		DX12DescriptorChunk view_desc;
		if (desc.view == ViewType::RenderTarget)
			view_desc = m_descriptor_mgr->allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		else if (desc.view == ViewType::DepthStencil)
			view_desc = m_descriptor_mgr->allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		else
			view_desc = m_descriptor_mgr->allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		if (desc.view == ViewType::DepthStencil)
		{
			auto dsv = to_dsv(desc.range);
			m_device->CreateDepthStencilView(tex_storage.resource.Get(), &dsv, view_desc.cpu_handle(0));
		}
		else if (desc.view == ViewType::RenderTarget)
		{
			auto rtv = to_rtv(desc.range);
			m_device->CreateRenderTargetView(tex_storage.resource.Get(), &rtv, view_desc.cpu_handle(0));
		}
		else if (desc.view == ViewType::ShaderResource)
		{
			auto srv = to_srv(desc.range);
			m_device->CreateShaderResourceView(tex_storage.resource.Get(), &srv, view_desc.cpu_handle(0));
		}
		else if (desc.view == ViewType::UnorderedAccess)
		{
			auto uav = to_uav(desc.range);
			m_device->CreateUnorderedAccessView(tex_storage.resource.Get(), nullptr, &uav, view_desc.cpu_handle(0));
		}
		else
		{
			assert(false);
		}

		try_insert(m_texture_views, TextureView_Storage(texture, desc.view, view_desc), get_slot(handle.handle));
	}
	

	u32 RenderDevice_DX12::get_global_descriptor(Buffer buffer, u32 view) const
	{
		//return (u32)try_get(m_buffers, get_slot(buffer.handle)).views[view].view.index_offset_from_base();
		return 0;
	}

	u32 RenderDevice_DX12::get_global_descriptor(Texture texture, u32 view) const
	{
		//return (u32)try_get(m_textures, get_slot(texture.handle)).views[view].view.index_offset_from_base();
		return 0;

		assert(false);
		return 0;
	}

	RenderCommandList* RenderDevice_DX12::allocate_command_list(QueueType queue)
	{
		HRESULT hr{ S_OK };

		D3D12_COMMAND_LIST_TYPE type{ D3D12_COMMAND_LIST_TYPE_DIRECT };
		std::queue<std::unique_ptr<RenderCommandList_DX12>>* cmd_list_pool{ nullptr };
		switch (queue)
		{
		case QueueType::Graphics:
			type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			cmd_list_pool = &m_cmd_list_pool_direct;
			break;
		case QueueType::Compute:
			type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
			cmd_list_pool = &m_cmd_list_pool_compute;
			break;
		case QueueType::Copy:
			type = D3D12_COMMAND_LIST_TYPE_COPY;
			cmd_list_pool = &m_cmd_list_pool_copy;
			break;
		default:
			cmd_list_pool = &m_cmd_list_pool_direct;
		}
		
		std::unique_ptr<RenderCommandList_DX12> render_cmd_list;
		if (!cmd_list_pool->empty())
		{
			render_cmd_list = std::move(cmd_list_pool->front());
			cmd_list_pool->pop();
		}
		else
		{
			ComPtr<ID3D12CommandAllocator> ator;
			ComPtr<ID3D12GraphicsCommandList4> cmdl;
			hr = m_device->CreateCommandAllocator(type, IID_PPV_ARGS(ator.GetAddressOf()));
			HR_VFY(hr);
			hr = m_device->CreateCommandList1(0, type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(cmdl.GetAddressOf()));
			HR_VFY(hr);
			
			render_cmd_list = std::make_unique<RenderCommandList_DX12>(this, ator, cmdl, queue);
		}
		auto ret = render_cmd_list.get();

		// store internally
		m_cmd_lists_in_play[ret] = std::move(render_cmd_list);

		ret->open();

		return ret;
	}

	void RenderDevice_DX12::submit_command_lists(std::span<RenderCommandList*> lists, QueueType queue, std::optional<SyncReceipt> incoming_sync, std::optional<SyncReceipt> outgoing_sync)
	{
		ID3D12CommandList* cmdls[16];
		for (u32 i = 0; i < lists.size(); ++i)
		{
			auto list = (RenderCommandList_DX12*)lists[i];
			list->close();

			auto [_, cmd_list] = list->get_allocator_and_list();
			cmdls[i] = cmd_list;
		}

		DX12Queue* curr_queue = get_queue(queue);

		// Use sync receipt to check up sync handle and synchronize accordingly (Fence Wait) before executing command list
		if (incoming_sync.has_value())
		{
			// Lookup sync, and perform GPU wait
			const auto& sync = try_get(m_syncs, get_slot(incoming_sync->handle));
			sync.fence.gpu_wait(*curr_queue);
		}

		curr_queue->execute_command_lists((u32)lists.size(), cmdls);

		std::optional<SyncReceipt> sync_receipt{ std::nullopt };
		if (outgoing_sync.has_value())
		{
			SyncPrimitive sync{};

			// Use recycled syncs
			if (!m_recycled_syncs.empty())
			{
				sync = std::move(m_recycled_syncs.front());
				m_recycled_syncs.pop();
			}
			// Create new sync
			else
			{
				sync.fence = DX12Fence(m_device.Get(), 0);
			}

			curr_queue->insert_signal(sync.fence);

			try_insert(m_syncs, std::move(sync), get_slot(outgoing_sync->handle));
		}
	}

	void RenderDevice_DX12::recycle_sync(SyncReceipt receipt)
	{
		auto sync = std::move(try_get(m_syncs, get_slot(receipt.handle)));
		m_recycled_syncs.push(sync);
		
		// mark as empty
		m_syncs[get_slot(receipt.handle)] = std::nullopt;
	}

	void RenderDevice_DX12::recycle_command_list(RenderCommandList* list)
	{
		auto api_list = std::move(m_cmd_lists_in_play[(RenderCommandList_DX12*)list]);

		switch (api_list->queue_type())
		{
		case QueueType::Graphics:
			m_cmd_list_pool_direct.push(std::move(api_list));
			break;
		case QueueType::Compute:
			m_cmd_list_pool_compute.push(std::move(api_list));
			break;
		case QueueType::Copy:
			m_cmd_list_pool_copy.push(std::move(api_list));
			break;
		default:
			break;
		}
	}

	void RenderDevice_DX12::upload_to_buffer(Buffer buffer, u32 dst_offset, void* data, u32 size)
	{
		const auto& res = try_get(m_buffers, get_slot(buffer.handle));

		// Ensure buffer is mappable
		assert(res.mapped_resource != nullptr);

		// Space can accomodate the copy
		assert((res.desc.size - dst_offset) >= size);

		std::memcpy(res.mapped_resource + dst_offset, data, size);
	}

	void RenderDevice_DX12::flush()
	{
		m_direct_queue->flush();
		m_compute_queue->flush();
		m_direct_queue->flush();
	}

	void RenderDevice_DX12::wait_for_gpu(SyncReceipt receipt)
	{
		const auto& sync = try_get(m_syncs, get_slot(receipt.handle));
		sync.fence.cpu_wait();

		recycle_sync(receipt);
	}






	ID3D12Resource* RenderDevice_DX12::get_api_buffer(Buffer buffer) const
	{
		return try_get(m_buffers, get_slot(buffer.handle)).resource.Get();
	}

	ID3D12Resource* RenderDevice_DX12::get_api_texture(Texture texture) const
	{
		return try_get(m_textures, get_slot(texture.handle)).resource.Get();
	}

	u32 RenderDevice_DX12::get_api_buffer_size(Buffer buffer) const
	{
		return try_get(m_buffers, get_slot(buffer.handle)).desc.size;
	}

	D3D12_RESOURCE_STATES RenderDevice_DX12::get_resource_state(ResourceState state) const
	{
		return to_internal(state);
	}

	D3D_PRIMITIVE_TOPOLOGY RenderDevice_DX12::get_api_topology(Pipeline pipeline) const
	{
		return try_get(m_pipelines, get_slot(pipeline.handle)).topology;
	}

	ID3D12PipelineState* RenderDevice_DX12::get_api_pipeline(Pipeline pipeline) const
	{
		return try_get(m_pipelines, get_slot(pipeline.handle)).pipeline.Get();
	}

	ID3D12RootSignature* RenderDevice_DX12::get_api_global_rsig() const
	{
		return m_common_rsig.Get();
	}

	ID3D12DescriptorHeap* RenderDevice_DX12::get_api_global_resource_dheap() const
	{
		return m_descriptor_mgr->get_gpu_dh_resource();
	}

	ID3D12DescriptorHeap* RenderDevice_DX12::get_api_global_sampler_dheap() const
	{
		return m_descriptor_mgr->get_gpu_dh_sampler();
	}

	const std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC>& RenderDevice_DX12::get_rp_rts(RenderPass rp) const
	{
		return try_get(m_renderpasses, get_slot(rp.handle)).render_targets;
	}

	std::optional<D3D12_RENDER_PASS_DEPTH_STENCIL_DESC> RenderDevice_DX12::get_rp_depth_stencil(RenderPass rp) const
	{
		return try_get(m_renderpasses, get_slot(rp.handle)).depth_stencil;
	}

	D3D12_RENDER_PASS_FLAGS RenderDevice_DX12::get_rp_flags(RenderPass rp) const
	{
		return try_get(m_renderpasses, get_slot(rp.handle)).flags;
	}

	ID3D12CommandQueue* RenderDevice_DX12::get_queue(D3D12_COMMAND_LIST_TYPE type)
	{
		switch (type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			return *m_direct_queue;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return *m_compute_queue;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			return *m_copy_queue;
		default:
			return *m_direct_queue;
		}
	}

	void RenderDevice_DX12::register_swapchain_texture(ComPtr<ID3D12Resource> texture, Texture handle)
	{
		Texture_Storage storage{};
		storage.resource = texture;
		auto desc = texture->GetDesc();

		try_insert(m_textures, storage, get_slot(handle.handle));
	}

	void RenderDevice_DX12::set_clear_color(Texture tex, const std::array<float, 4>& clear_color)
	{
		auto& res = try_get(m_textures, get_slot(tex.handle));
		res.desc.clear_color = clear_color;
	}

	void RenderDevice_DX12::create_queues()
	{
		m_direct_queue = std::make_unique<DX12Queue>(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
		m_copy_queue = std::make_unique<DX12Queue>(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COPY);
		m_compute_queue = std::make_unique<DX12Queue>(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
	}

	void RenderDevice_DX12::init_dma(IDXGIAdapter* adapter)
	{
		HRESULT hr{ S_OK };

		D3D12MA::ALLOCATOR_DESC desc{};
		desc.pDevice = m_device.Get();
		desc.pAdapter = adapter;
		hr = D3D12MA::CreateAllocator(&desc, m_dma.GetAddressOf());
		HR_VFY(hr);
	}

	void RenderDevice_DX12::init_rootsig()
	{
		HRESULT hr{ S_OK };

		const u8 num_constants = 8;

		std::vector<D3D12_ROOT_PARAMETER> params;
		for (u32 reg = 0; reg < num_constants; ++reg)
		{
			D3D12_ROOT_PARAMETER param{};
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			param.Constants.RegisterSpace = 0;
			param.Constants.ShaderRegister = reg;
			param.Constants.Num32BitValues = 1;

			// Indirect Set Constant requires 2 for some reason to start working with Debug Validation Layer is on
			if (reg == 0)
				param.Constants.Num32BitValues = 2;

			params.push_back(param);
		}

		D3D12_ROOT_SIGNATURE_DESC rsd{};
		rsd.NumParameters = (UINT)params.size();
		rsd.pParameters = params.data();
		rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

		auto static_samplers = grab_static_samplers();
		rsd.NumStaticSamplers = (u32)static_samplers.size();
		rsd.pStaticSamplers = static_samplers.data();

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		hr = D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		if (FAILED(hr))
		{
			std::string msg = "Compilation failed with errors:\n";
			msg += std::string((const char*)error->GetBufferPointer());

			std::cout << msg.c_str() << "\n";
			assert(false);
		}

		hr = m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_common_rsig.GetAddressOf()));
		HR_VFY(hr);
	}

	std::vector<D3D12_STATIC_SAMPLER_DESC> RenderDevice_DX12::grab_static_samplers()
	{
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
		constexpr uint32_t space = 1;
		uint32_t next_register = 0;
		{
			D3D12_STATIC_SAMPLER_DESC sd1{};
			sd1.Filter = D3D12_FILTER_ANISOTROPIC;
			sd1.AddressU = sd1.AddressV = sd1.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sd1.MipLODBias = 0.f;
			sd1.MaxAnisotropy = 8;
			sd1.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			sd1.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
			sd1.MinLOD = 0.f;
			sd1.MaxLOD = D3D12_FLOAT32_MAX;
			sd1.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			sd1.ShaderRegister = next_register++;
			sd1.RegisterSpace = space;
			samplers.push_back(sd1);
		}

		{
			D3D12_STATIC_SAMPLER_DESC sd1{};
			sd1.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			sd1.AddressU = sd1.AddressV = sd1.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sd1.MipLODBias = 0.f;
			sd1.MaxAnisotropy = 8;
			sd1.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			sd1.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
			sd1.MinLOD = 0.f;
			sd1.MaxLOD = D3D12_FLOAT32_MAX;
			sd1.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			sd1.ShaderRegister = next_register++;
			sd1.RegisterSpace = space;
			samplers.push_back(sd1);
		}

		return samplers;
	}

	DX12Queue* RenderDevice_DX12::get_queue(QueueType type)
	{
		DX12Queue* curr_queue{ nullptr };
		switch (type)
		{
		case QueueType::Graphics:
			curr_queue = m_direct_queue.get();
			break;
		case QueueType::Compute:
			curr_queue = m_compute_queue.get();
			break;
		case QueueType::Copy:
			curr_queue = m_copy_queue.get();
			break;
		default:
			curr_queue = m_direct_queue.get();
		}
		return curr_queue;
	}


}

