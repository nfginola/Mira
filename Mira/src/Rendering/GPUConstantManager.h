#pragma once
#include "../Common.h"
#include "../Memory/VirtualBlockAllocator.h"
#include "../Memory/RingBuffer.h"
#include "../Memory/BumpAllocator.h"
#include "../RHI/RenderResourceHandle.h"
#include "../RHI/RenderCommandList.h"
#include "../Handles/HandleAllocator.h"
#include "Types/GPUConstantTypes.h"

#include <queue>

namespace mira
{
	class RenderDevice;
	class GPUGarbageBin;

	/*
		Handles GPU constant data management
	
		- CPU write-once, GPU read-once --> transient; lives on Upload; 
		- CPU write-once, GPU read-many --> persistent; lives on Default;

		Update of persistent buffer data is VERSIONED. This means that every time an update occurs
		on a persistent constant, it WILL change storage.
		
		Allows 256, 512, 1024 allocations.
	
	*/
	class GPUConstantManager
	{
	public:
		GPUConstantManager(RenderDevice* rd, GPUGarbageBin* bin, u8 max_versions);

		// Transient constants live in shared host-device memory
		// User can immediately update on CPU
		std::pair<u8*, u32> allocate_transient(u32 size);

		// Persistent (lives in device-local memory)
		PersistentConstant allocate_persistent(u32 size, void* init_data = nullptr, u32 init_data_size = 0, bool immutable = false);
		void free_persistent(PersistentConstant handle);
		u32 get_global_view(PersistentConstant handle) const;					// Grab GPU-indexable handle

		void upload(PersistentConstant handle, void* data, u32 size);				// Enqueue upload request for persistent constants

		// Executes enqueued GPU-GPU copies if any.
		// Do note that the sync receipt is consumed internally upon Version wrap-around! (execute copies updates Version)
		// Use for external sync with caution.
		std::optional<SyncReceipt> execute_copies(std::optional<SyncReceipt> read_sync = {}, bool generate_sync = false, QueueType submit_queue = QueueType::Graphics);



	private:
		struct Persistent_Buffer
		{
			Buffer buffer;
			VirtualBlockAllocator ator;
		};

		struct Transient_Buffer
		{
			Buffer buffer;
			RingBuffer ator;
		};

		struct Staging_Buffer
		{
			Buffer buffer;
			BumpAllocator ator;
		};

		/*
			When user requestes an upload to persistent:
				Allocation and view is safely removed
				and they are populated with the new version
		*/
		struct PersistentConstant_Storage
		{
			u8 curr_version{ 0 };					
			std::pair<u64, u64> allocation_md;
			BufferView view;
			std::function<void(RenderCommandList&)> copy_request_func;

			bool immutable{ false };		// Disables updating
			u32 allocated_size{ 0 };
		};

	private:
		void upload_persistent_to_device_local(PersistentConstant handle, PersistentConstant_Storage& storage, void* data, u32 data_size);

	private:
		RenderDevice* m_rd{ nullptr };
		GPUGarbageBin* m_bin{ nullptr };

		// Tracks persistent constants that require updating
		std::queue<PersistentConstant> m_persistents_with_copy_requests;

		// Persistent
		std::vector<Persistent_Buffer> m_persistent_buffers;					// One per VERSION; 256/512/1024 allowed from single buffer (pool allocator)
		std::vector<Staging_Buffer> m_persistent_stagings;						// One per VERSION;
		std::vector<std::optional<SyncReceipt>> m_staging_to_dl_syncs;			// One per VERSION;
		std::vector<std::optional<CommandList>> m_copy_cmdls;					// One per VERSION; Held to recycle
		u8 m_max_versions{ 0 };
		u8 m_curr_version{ 0 };

		std::vector<std::optional<PersistentConstant_Storage>> m_persistent_allocations;
		HandleAllocator m_handle_ator;

		// Transient 
		/*
			Uses a single buffer for all 256, 512, 1024 allocations
			RingBuffer element is 256 large
			--> 512 byte allocations are 2 elements and 1024 byte allocations are 3 elements
		*/
		Transient_Buffer m_transient_buffer;

	};
}


