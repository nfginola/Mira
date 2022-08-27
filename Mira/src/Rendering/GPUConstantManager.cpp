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
		m_copy_cmdls.resize(max_versions);

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
	}

	PersistentConstant GPUConstantManager::allocate_persistent(u32 size, void* init_data, u32 init_data_size, bool immutable)
	{
		assert(size <= 1024);
		assert(init_data_size <= size);

		PersistentConstant_Storage storage{};
		storage.immutable = immutable;
		storage.allocated_size = (1 + ((size - 1) / 256)) * 256;		// round up to nearest multiple of 256

		auto handle = m_handle_ator.allocate<PersistentConstant>();

		upload_persistent_to_device_local(handle, storage, init_data, init_data_size);

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

	void GPUConstantManager::upload(PersistentConstant handle, void* data, u32 size)
	{
		auto& res = try_get(m_persistent_allocations, get_slot(handle.handle));

		// User trying to update constant that they marked as immutable
		assert(!res.immutable);

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

		upload_persistent_to_device_local(handle, res, data, size);
	}

	std::optional<SyncReceipt> GPUConstantManager::execute_copies(std::optional<SyncReceipt> read_sync, bool generate_sync, QueueType submit_queue)
	{
		RenderCommandList list;
		
		// CPU wait until staging-to-device copies for this version are finished
		if (m_staging_to_dl_syncs[m_curr_version].has_value())
		{
			m_rd->wait_for_gpu(*m_staging_to_dl_syncs[m_curr_version]);
			m_staging_to_dl_syncs[m_curr_version] = std::nullopt;
		}

		// Staging to Device Local for current version guaranteed to be complete on the GPU --> Free to clear
		m_persistent_stagings[m_curr_version].ator.clear();

		// Recycle the command list previously used for this version copy
		if (m_copy_cmdls[m_curr_version].has_value())
			m_rd->recycle_command_list(*m_copy_cmdls[m_curr_version]);

		// Record all copy requests since previous execute copy
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

		// Execute
		CommandList cmdls[]{ m_rd->allocate_command_list(submit_queue) };
		m_rd->compile_command_list(cmdls[0], list);

		// Read sync is to guarantee read completion of this Version before we proceed to copy to it. (critical if read occurs on different queue!)
		auto receipt = m_rd->submit_command_lists(cmdls, submit_queue, read_sync, true);

		// Keep cmdl for safe recycling on next wrap-around
		m_copy_cmdls[m_curr_version] = cmdls[0];

		// Always keep sync (this is used to know when staging buffer can appropriately be cleared)
		m_staging_to_dl_syncs[m_curr_version] = *receipt;

		// Advance version
		m_curr_version = (m_curr_version + 1) % m_max_versions;

		// Outside components can sync if they request for it
		return generate_sync ? receipt : std::nullopt;
	}
	
	void GPUConstantManager::upload_persistent_to_device_local(PersistentConstant handle, PersistentConstant_Storage& storage, void* data, u32 data_size)
	{
		// Set new version
		storage.curr_version = m_curr_version;

		// Re-allocate memory
		auto dl_offset = m_persistent_buffers[m_curr_version].ator.allocate(storage.allocated_size);
		assert(dl_offset != (u64)-1);
		storage.allocation_md = { dl_offset, storage.allocated_size };


		// Create view for new memory
		storage.view = m_rd->create_view(m_persistent_buffers[m_curr_version].buffer, BufferViewDesc(ViewType::Constant, (u32)dl_offset, storage.allocated_size));

		// Set GPU-GPU copy request
		storage.copy_request_func = [this, &storage, data, data_size, dl_offset](RenderCommandList& list)
		{
			// Copy data to staging memory
			auto [staging, staging_offset] = m_persistent_stagings[m_curr_version].ator.allocate_with_offset(data_size);
			std::memcpy(staging, data, data_size);

			list.submit(RenderCommandCopyBuffer(
				m_persistent_stagings[m_curr_version].buffer, staging_offset,
				m_persistent_buffers[m_curr_version].buffer, dl_offset,
				data_size));
		};

		m_persistents_with_copy_requests.push(handle);
	}
}
