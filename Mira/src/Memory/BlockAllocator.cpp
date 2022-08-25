#include "BlockAllocator.h"

namespace mira
{
	BlockAllocator::BlockAllocator(u32 block_size, u32 block_count, u8* memory) :
		m_block_size(block_size),
		m_block_count(block_count),
		m_internally_managed_memory(memory == nullptr ? true : false)
	{
		const u64 total_size = (u64)block_size * block_count;
		if (m_internally_managed_memory)
			m_heap_start = (u8*)std::malloc(total_size);
		else
			m_heap_start = memory;

		m_heap_end = m_heap_start + total_size;
		
		assert(m_heap_start != nullptr);
		std::memset(m_heap_start, 0, total_size);
	}

	BlockAllocator::~BlockAllocator()
	{
		if (m_internally_managed_memory)
			std::free(m_heap_start);
	}

	[[nodiscard]] u8* BlockAllocator::allocate(u64 size)
	{
		const u64 offset = m_vator.allocate(size);
		assert(offset != (u64)-1);

		return m_heap_start + offset;
	}

	void BlockAllocator::free(u64 offset, u64 size)
	{
		m_vator.free(offset, size);
	}
}

