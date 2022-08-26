#include "GPUConstantManager.h"
#include "../RHI/RenderDevice.h"
#include "GPUGarbageBin.h"

namespace mira
{
	GPUConstantManager::GPUConstantManager(RenderDevice* rd, GPUGarbageBin* bin, u8 max_versions) :
		m_rd(rd),
		m_bin(bin),
		m_max_versions(max_versions)
	{
		m_persistent_allocations.resize(1);
		m_staging_to_dl_syncs.resize(max_versions);

		m_persistent_buffers.resize(max_versions);
		m_persistent_stagings.resize(max_versions);

		for (u32 version = 0; version < max_versions; ++version)
		{
			// Init persistent buffer
			m_persistent_buffers[version].buffer = m_rd->create_buffer(BufferDesc(256'000, MemoryType::Default));
			m_persistent_buffers[version].ator = VirtualBlockAllocator(256, 256'000 / 256);
			
			// Init staging
			m_persistent_stagings[version].buffer = m_rd->create_buffer(BufferDesc(256'000, MemoryType::Upload));
			m_persistent_stagings[version].ator = BumpAllocator(256'000, m_rd->map(m_persistent_stagings[version].buffer));
		}

		// Setup dynamic constant buffer
		{
			constexpr u32 MAX_DYN_CB_SIZE = 256'000;
			m_transient_buffer.buffer = m_rd->create_buffer(BufferDesc(MAX_DYN_CB_SIZE, MemoryType::Upload));
			m_transient_buffer.ator = RingBuffer(256, MAX_DYN_CB_SIZE / 256, m_rd->map(m_transient_buffer.buffer));
		}
	}

	std::pair<u8*, u32> GPUConstantManager::allocate_transient(u32 size)
	{
		assert(size <= 1024);
		
		u32 elements_required = 1 + ((size - 1) / 256);	

		u8* allocation{ nullptr };
		u64 allocation_offset{ 0 };
		bool contiguous_satisfied{ true };
		for (u32 i = 0; i < elements_required; ++i)
		{
			// Start
			if (i == 0)
			{
				auto [alloc, offset] = m_transient_buffer.ator.allocate_with_offset();
				allocation = alloc;
				allocation_offset = offset;
			}
			else
			{
				// Reserve more elements
				if (!m_transient_buffer.ator.allocate())
					contiguous_satisfied = false;
			}
		}

		assert(allocation != nullptr);
		assert(contiguous_satisfied);		// Out of memory, just increase max memory

		// Create transient GPU-indexable view
		auto view = m_rd->create_view(m_transient_buffer.buffer, BufferViewDesc(ViewType::Constant, (u32)allocation_offset, elements_required * 256));
		auto global_id = m_rd->get_global_descriptor(view);

		// Free this view when appropriate
		m_bin->push_deferred_deletion([this, view, elements_required]()
			{
				m_rd->free_view(view);

				// Pop the allocated elements
				for (u32 i = 0; i < elements_required; ++i)
					m_transient_buffer.ator.pop();
			});

		return { allocation, global_id };


		/*
			Old code when we used one buffer per size for dynamic data
		*/
		//// Find suitable buffer
		//u8* allocation{ nullptr };
		//u64 allocation_offset{ 0 };
		//u32 allocation_size{ 0 };
		//Buffer suitable_buffer;
		//for (u32 i = 0; i < m_transient_buffers.size(); ++i)
		//{
		//	auto& ator = m_transient_buffers[i].ator;
		//	auto& buffer = m_transient_buffers[i].buffer;

		//	if (size <= ator.get_element_size())
		//	{
		//		auto [alloc, offset] = ator.allocate_with_offset();

		//		allocation = alloc;
		//		allocation_offset = offset;
		//		allocation_size = ator.get_element_size();
		//		suitable_buffer = buffer;
		//		
		//		// Allocation success
		//		if (allocation)
		//			break;
		//	}
		//}

		//assert(allocation != nullptr);

		//// Create transient GPU-indexable view
		//auto view = m_rd->create_view(suitable_buffer, BufferViewDesc(ViewType::Constant, (u32)allocation_offset, allocation_size));
		//auto global_id = m_rd->get_global_descriptor(view);

		//// Free this view when appropriate
		//m_bin->push_deferred_deletion([this, view]()
		//	{
		//		m_rd->free_view(view);
		//	});

		//return { allocation, global_id };
	}

	PersistentConstant GPUConstantManager::allocate_persistent(u32 size, u8* init_data, u32 init_data_size, bool immutable)
	{
		assert(size % 256 == 0);
		assert(size <= 1024);
		assert(init_data_size <= size);

		PersistentConstant_Storage storage{};
		storage.immutable = immutable;
		storage.allocated_size = (1 + ((size - 1) / 256)) * 256;		// round up to nearest multiple of 256

		auto handle = m_handle_ator.allocate<PersistentConstant>();

		// #1
		{
			// Set version
			storage.curr_version = m_curr_version;

			// Allocate memory
			auto dl_offset = m_persistent_buffers[m_curr_version].ator.allocate(storage.allocated_size);
			assert(dl_offset != (u64)-1);
			storage.allocation_md = { dl_offset, storage.allocated_size };

			// Create view for new memory
			storage.view = m_rd->create_view(m_persistent_buffers[m_curr_version].buffer, BufferViewDesc(ViewType::Constant, (u32)dl_offset, storage.allocated_size));

			if (init_data && init_data_size != 0)
			{
				// Set GPU-GPU copy request
				storage.copy_request_func = [this, init_data, init_data_size, dl_offset, size](RenderCommandList& list)
				{
					// Copy data to staging memory
					auto [staging, staging_offset] = m_persistent_stagings[m_curr_version].ator.allocate_with_offset(size);
					std::memcpy(staging, init_data, init_data_size);

					list.submit(RenderCommandCopyBuffer(
						m_persistent_stagings[m_curr_version].buffer, staging_offset,
						m_persistent_buffers[m_curr_version].buffer, dl_offset,
						size));
				};

				
				m_persistents_with_copy_requests.push(handle);
			}
		}

		try_insert(m_persistent_allocations, storage, get_slot(handle.handle));

		return handle;
	}

	void GPUConstantManager::free_persistent(PersistentConstant handle)
	{
		auto& res = try_get(m_persistent_allocations, get_slot(handle.handle));

		// Safely remove current persistent allocation
		m_bin->push_deferred_deletion([this, res_copy = res, handle]()		// Grabs copy of res
			{
				m_persistent_buffers[res_copy.curr_version].ator.free(res_copy.allocation_md.first, res_copy.allocation_md.second);
				m_rd->free_view(res_copy.view);
			});
		
		// Deletion lambda has a copy of storage for deallocation, we can disable it immediately.
		m_persistent_allocations[get_slot(handle.handle)] = std::nullopt;
		m_handle_ator.free(handle);
	}

	u32 GPUConstantManager::get_global_view(PersistentConstant handle) const
	{
		const auto& res = try_get(m_persistent_allocations, get_slot(handle.handle));
		return m_rd->get_global_descriptor(res.view);
	}

	void GPUConstantManager::upload(PersistentConstant handle, u8* data, u32 size)
	{
		auto& res = try_get(m_persistent_allocations, get_slot(handle.handle));

		// User trying to update constant that they marked as immutable
		if (res.immutable)
			assert(false);		

		// User already requested an upload and haven't executed the copies yet!
		assert(!res.copy_request_func);

		// Nothing to copy..
		assert(data != nullptr);
		assert(size != 0);
		
		// User trying to copy more data than possible
		assert(size <= res.allocated_size);	

		// Safely remove current persistent allocation
		m_bin->push_deferred_deletion([this, res_copy = res, handle]()		// Grabs copy of res
			{
				m_persistent_buffers[res_copy.curr_version].ator.free(res_copy.allocation_md.first, res_copy.allocation_md.second);
				m_rd->free_view(res_copy.view);
			});

		// #1
		{
			// Set new version
			res.curr_version = m_curr_version;

			// Re-allocate memory
			auto dl_offset = m_persistent_buffers[m_curr_version].ator.allocate(res.allocated_size);
			assert(dl_offset != (u64)-1);
			res.allocation_md = { dl_offset, res.allocated_size };

			// Create view for new memory
			res.view = m_rd->create_view(m_persistent_buffers[m_curr_version].buffer, BufferViewDesc(ViewType::Constant, (u32)dl_offset, res.allocated_size));

			// Set GPU-GPU copy request
			res.copy_request_func = [this, &res, data, size, dl_offset](RenderCommandList& list)
			{
				// Copy data to staging memory
				auto [staging, staging_offset] = m_persistent_stagings[m_curr_version].ator.allocate_with_offset(size);
				std::memcpy(staging, data, size);

				list.submit(RenderCommandCopyBuffer(
					m_persistent_stagings[m_curr_version].buffer, staging_offset,
					m_persistent_buffers[m_curr_version].buffer, dl_offset,
					size));
			};

			m_persistents_with_copy_requests.push(handle);
		}
	}

	std::optional<SyncReceipt> GPUConstantManager::execute_copies(bool generate_sync)
	{
		RenderCommandList list;

		// If copies done on current version has not finished on the GPU, wait..
		/*
			We are constraining here that a Versioned copy can be dispatched only one at a time.

			Essentially: 
				If copies to Version N has not finished on the GPU,
				copy requests to Version N on the CPU is halted until it is finished.

		*/
		m_rd->wait_for_gpu(m_staging_to_dl_syncs[m_curr_version]);
		m_persistent_stagings[m_curr_version].ator.clear();			// Staging to Device Local for current version guaranteed to be complete on the GPU --> Free to clear

		// Clear all copy requests since previous execute copy
		while (!m_persistents_with_copy_requests.empty())
		{
			auto handle = m_persistents_with_copy_requests.front();
			auto& res = try_get(m_persistent_allocations, get_slot(handle.handle));
			m_persistents_with_copy_requests.pop();

			// Record GPU-GPU copy request
			res.copy_request_func(list);
			
			// Clear request
			res.copy_request_func = {};

			assert(res.curr_version == m_curr_version);
		}

		if (list.empty())
			return {};

		// On graphics queue for now to avoid any Copy -> Graphics queue sync
		QueueType queue_copy_dest = QueueType::Graphics;

		CommandList cmdls[]{ m_rd->allocate_command_list(queue_copy_dest) };
		m_rd->compile_command_list(cmdls[0], list);
		auto receipt = m_rd->submit_command_lists(cmdls, queue_copy_dest, {}, true);

		// Always keep sync (this is used know when staging buffer can be cleared)
		m_staging_to_dl_syncs[m_curr_version] = *receipt;

		// Advance version
		m_curr_version = (m_curr_version + 1) % m_max_versions;

		return generate_sync ? receipt : std::nullopt;
	}
}
