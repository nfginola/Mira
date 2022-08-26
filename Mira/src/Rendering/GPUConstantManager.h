#pragma once
#include "../Common.h"

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
	
	*/
	class GPUConstantManager
	{
	public:

		/*
			[memory, global_view_idx] = allocate_transient(size);				// Fire and forget, view destruction pushed to Garbage Bin

			[PersistentConstant, global_view_idx] = allocate_persistent(size);	// Allocate and keep

			upload(PersistentConstant handle, u8* data, u32 size);				// Do copies on Graphics queue so we don't need to GPU sync? (Synchronous copy)

			// If we do want to have explicit sync then..:

			void begin_copy();
			.. do copies..
			void end_copy();

			std::optional<SyncReceipt> execute_copies(bool generate_sync);

			free_persistent(PersistentConstant handle);							// Push to Garbage Bin
		*/


	private:
		

	};
}


