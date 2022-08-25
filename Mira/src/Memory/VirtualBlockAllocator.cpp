#include "VirtualBlockAllocator.h"

namespace mira
{
	VirtualBlockAllocator::VirtualBlockAllocator(u32 block_size, u32 block_count) :
		m_block_size(block_size),
		m_block_count(block_count)
	{
		m_total_size = block_size * block_count;
		m_block_states = (u8*)std::malloc(block_count);
		assert(m_block_states != nullptr);
		std::memset(m_block_states, 0, block_count);
	}

	VirtualBlockAllocator::~VirtualBlockAllocator()
	{
		std::free(m_block_states);
	}

	u64 VirtualBlockAllocator::allocate(u64 size)
	{
		const u32 count = (u32)((size - 1) / m_block_size) + 1;

		const u32 block_idx = find_contiguous_blocks(count);
		if (block_idx == m_block_count)
			return -1;

		set_blocks_state(block_idx, count, STATE_OCCUPIED);
		return block_idx * m_block_size;
	}

	void VirtualBlockAllocator::free(u64 offset, u64 size)
	{
		const u32 block_idx = (u32)(offset / m_block_size);
		const u32 count = (u32)(((size - 1) / m_block_size) + 1);
		set_blocks_state(block_idx, count, STATE_AVAILABLE);
	}

	u32 VirtualBlockAllocator::find_contiguous_blocks(u32 count)
	{
		for (u32 block = 0; block < m_block_count; ++block)
		{
			if (m_block_states[block] == STATE_OCCUPIED)
				continue;

			bool contiguous{ true };
			for (u32 contiguous_block = 1; contiguous_block < count; ++contiguous_block)
			{
				if (m_block_states[block + contiguous_block] == STATE_OCCUPIED)
				{
					contiguous = false;
					break;
				}
			}

			if (contiguous)
				return block;
		}

		// Not found
		return m_block_count;
	}

	void VirtualBlockAllocator::set_blocks_state(u32 offset, u32 count, u8 state)
	{
		for (u32 i = 0; i < count; ++i)
			m_block_states[offset + i] = state;
	}
}

