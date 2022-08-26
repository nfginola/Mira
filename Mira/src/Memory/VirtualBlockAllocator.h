#pragma once
#include "../Common.h"
#include <bitset>

namespace mira
{
	class VirtualBlockAllocator
	{
	public:	
		VirtualBlockAllocator() = default;
		VirtualBlockAllocator(u32 block_size, u32 block_count);
		~VirtualBlockAllocator();
		
		// Grabs contiguous blocks which fits at least the requested size
		[[nodiscard]] u64 allocate(u64 size);

		// User needs to keep track of allocation size
		void free(u64 offset, u64 size);

	private:
		static constexpr u8 STATE_AVAILABLE{ 0 };
		static constexpr u8 STATE_OCCUPIED{ 1 };

	private:
		// Finds block start index to the requested contiguous blocks
		u32 find_contiguous_blocks(u32 count);
		void set_blocks_state(u32 offset, u32 count, u8 state);

	private:
		u64 m_total_size{ 0 };
		u32 m_block_size{ 0 };
		u32 m_block_count{ 0 };

		std::vector<u8> m_block_states;
	};
}
