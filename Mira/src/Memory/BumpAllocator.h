#pragma once
#include "VirtualBumpAllocator.h"

namespace mira
{
	class BumpAllocator
	{
	public:
		BumpAllocator() = default;

		// Use externally managed memory
		BumpAllocator(u8* memory, u64 size) :
			m_vator(size),
			m_size(size),
			m_heap_start(memory),
			m_heap_end(memory + size),
			m_external_memory(true)
		{
			std::memset(m_heap_start, 0, size);
		}

		// Use internally managed memory
		BumpAllocator(u64 size) :
			m_vator(size),
			m_size(size),
			m_heap_start((u8*)std::malloc(size)),
			m_heap_end(m_heap_start + size),
			m_external_memory(false)
		{
			std::memset(m_heap_start, 0, size);
		}


		~BumpAllocator()
		{
			if (m_external_memory)
				std::free(m_heap_start);
		}

		[[nodiscard]] u8* allocate(u64 size, u16 alignment = 4)
		{
			u64 offset = m_vator.allocate(size, alignment);
			return m_heap_start + offset;
		}

		void clear()
		{
			m_vator.clear();
		}

	private:
		VirtualBumpAllocator m_vator;

		u64 m_size{ 0 };
		u8* m_heap_start{ nullptr };
		u8* m_heap_end{ nullptr };
		bool m_external_memory{ false };
	};
}


