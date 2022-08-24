#pragma once
#include "../Common.h"

namespace mira
{
	class VirtualBlockAllocator
	{
	public:	
		VirtualBlockAllocator();
		VirtualBlockAllocator(u32 block_size, u32 block_count);
		
		// Grabs contiguous blocks which fits at least the requested size
		u64 allocate(u64 size);

		// User needs to keep track of allocation size
		void free(u64 offset, u64 size);

	private:
		u64 m_total_size{ 0 };
		u32 m_block_size{ 0 };
		u32 m_block_count{ 0 };

		// Track used blocks (one bit per block)
		u8* m_ledger{ nullptr };
	};
}
