#pragma once
#include "../Common.h"
#include "../Memory/VirtualBlockAllocator.h"
#include "../Memory/BumpAllocator.h"
#include "../RHI/RenderResourceHandle.h"

namespace mira
{
	/*
		Handles GPU constant data management
	
		- CPU write-once, GPU read-once --> transient; lives on Upload; 
		- CPU write-once, GPU read-many --> persistent; lives on Default;

		Uses memory pool:
			256 byte pool
			512 byte pool
			1024 byte pool

		Transient and Persistent keeps a memory pool RESPECTIVELY.
		(Persistent also keeps a staging buffer for upload)

		Transient can use a single ring buffer which allocates 256, 512 or 1024 
	
	*/
	class GPUConstantManager
	{
	public:
		
		/*
			[memory, global_view_idx] = allocate_transient(size);				// Fire and forget, view destruction pushed to Garbage Bin

			[PersistentConstant] = allocate_persistent(size);	// Allocate and keep

			upload(PersistentConstant handle, u8* data, u32 size);
			--> upload could simply copy to staging buffers and push GPU-GPU copies onto a queue
			--> which is then consumed on "execute_copies" and actually translated to an ExecuteCommandList
			--> execute_copies essentially flushes the requested uploads so far.

			std::optional<SyncReceipt> execute_copies(bool generate_sync);		// Advances version system for persistent buffers!!!

			// Have to grab on bind time
			u32 get_view_idx(PersistentConstant) 

			free_persistent(PersistentConstant handle);							// Push to Garbage Bin
		*/

	private:
		struct DeviceLocal_Buffer
		{
			mira::Buffer buffer;
			mira::BufferView full_view;
			VirtualBlockAllocator ator;
		};

		struct Staging_Buffer
		{
			mira::Buffer buffer;
			BumpAllocator ator;
		};

	private:
		// Persistent
		Staging_Buffer m_persistent_staging;
		std::vector<DeviceLocal_Buffer> m_persistent_buffers;



		

	};
}


