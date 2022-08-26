#pragma once
#include "VirtualBlockAllocator.h"

namespace mira
{
	class PoolAllocator
	{
	public:
		struct BlockSpecification
		{
			u32 block_size{ 0 };
			u32 block_count{ 0 };
			void* memory{ nullptr };
		};

		struct SizeSpecification
		{
			std::vector<BlockSpecification> block_specs;
		};

	public:
		PoolAllocator(const SizeSpecification& size_spec);

		[[nodiscard]] u8* allocate(u64 size);
		void free(u8* memory, u64 size);

	private:
		// ID is the index of the stored VirtualBlockAllocator
		bool belongs(u32 id, void* memory);

	private:
		std::vector<VirtualBlockAllocator> m_blocks;
		std::vector<u8*> m_block_memory_starts;
	};
}



