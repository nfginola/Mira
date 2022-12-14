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

		u32 get_block_size() const { return m_block_size; }
		u64 get_total_size() const { return m_total_size; }

	private:
		//static constexpr u8 STATE_AVAILABLE{ 0 };
		//static constexpr u8 STATE_OCCUPIED{ 1 };

		static constexpr bool STATE_AVAILABLE{ false };
		static constexpr bool STATE_OCCUPIED{ true };

	private:
		// Finds block start index to the requested contiguous blocks
		u32 find_contiguous_blocks(u32 count);
		void set_blocks_state(u32 offset, u32 count, u8 state);

	private:
		u64 m_total_size{ 0 };
		u32 m_block_size{ 0 };
		u32 m_block_count{ 0 };

		std::vector<bool> m_block_states;
	};
}
