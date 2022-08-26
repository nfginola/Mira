#include "PoolAllocator.h"
#include <set>

namespace mira
{
	PoolAllocator::PoolAllocator(const SizeSpecification& spec)
	{
		m_block_memory_starts.resize(spec.block_specs.size());

		std::set<u32> block_sizes;
		for (u32 i = 0; i < spec.block_specs.size(); ++i)
		{
			const auto& block_spec = spec.block_specs[i];

			// Only unique block sizes are allowed!
			if (block_sizes.find(block_spec.block_size) != block_sizes.cend())
				assert(false);
			block_sizes.insert(block_spec.block_size);

			m_blocks.push_back(VirtualBlockAllocator(block_spec.block_size, block_spec.block_count));

			// Allocate memory internally if not provided
			if (!block_spec.memory)
				m_block_memory_starts[i] = (u8*)std::malloc(block_spec.block_count * block_spec.block_size);
		}
	}

	u8* PoolAllocator::allocate(u64 size)
	{
		// Find first fit
		u8* alloc{ nullptr };
		for (u32 i = 0; i < m_blocks.size(); ++i)
		{
			auto& block = m_blocks[i];
			// Found suitable block
			if (size <= block.get_block_size())
			{
				auto local_offset = block.allocate(size);

				// Allocation succeeded
				if (local_offset != (u32)-1)
				{
					const u64 global_offset = (u64)m_block_memory_starts[i] + local_offset;
					alloc = (u8*)global_offset;
				}
			}
		}

		return alloc;
	}

	void PoolAllocator::free(u8* memory, u64 size)
	{
		for (u32 i = 0; i < m_blocks.size(); ++i)
		{
			if (belongs(i, memory))
			{
				// get local offset
				u64 local_offset = (u64)memory - (u64)m_block_memory_starts[i];
				m_blocks[i].free(local_offset, size);
			}
		}
	}

	bool PoolAllocator::belongs(u32 id, void* memory)
	{
		const auto& block = m_blocks[id];
		u8* start = m_block_memory_starts[id];
		u8* end = (start + block.get_total_size());

		if (start <= memory && memory < end)
			return true;
		else
			return false;
	}
}
