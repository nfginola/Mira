#pragma once
#include "VirtualBlockAllocator.h"

namespace mira
{
	class BlockAllocator
	{
	public:
		BlockAllocator() = default;
		BlockAllocator(u32 block_size, u32 block_count, u8* memory = nullptr);
		~BlockAllocator();

		// Grabs contiguous blocks which fits at least the requested size
		u8* allocate(u64 size);

		// User needs to keep track of allocation size
		void free(u64 offset, u64 size);

	private:
		VirtualBlockAllocator m_vator;

		u32 m_block_size{ 0 };
		u32 m_block_count{ 0 };
		bool m_internally_managed_memory{ false };

		u8* m_heap_start{ nullptr };
		u8* m_heap_end{ nullptr };
	};
}

