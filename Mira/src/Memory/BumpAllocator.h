#pragma once
#include "VirtualBumpAllocator.h"

namespace mira
{
	class BumpAllocator
	{
	public:
		BumpAllocator() = default;

		BumpAllocator(u64 size, u8* memory = nullptr) :
			m_vator(size),
			m_size(size),
			m_internally_managed_memory(memory == nullptr ? true : false)
		{
			if (m_internally_managed_memory)
				m_heap_start = (u8*)std::malloc(size);
			else
				m_heap_start = memory;

			m_heap_end = m_heap_start + size;

			assert(m_heap_start != nullptr);
			std::memset(m_heap_start, 0, size);
		}

		~BumpAllocator()
		{
			if (m_internally_managed_memory)
				std::free(m_heap_start);
		}

		[[nodiscard]] u8* allocate(u64 size, u16 alignment = 0)
		{
			u64 offset = m_vator.allocate(size, alignment);
			return m_heap_start + offset;
		}

		// { memory, offset from base } 
		[[nodiscard]] std::pair<u8*, u64> allocate_with_offset(u64 size, u16 alignment = 0)
		{
			u64 offset = m_vator.allocate(size, alignment);
			return { m_heap_start + offset, offset };
		}

		void clear()
		{
			m_vator.clear();
		}

	private:
		VirtualBumpAllocator m_vator;

		u64 m_size{ 0 };
		bool m_internally_managed_memory{ false };

		u8* m_heap_start{ nullptr };
		u8* m_heap_end{ nullptr };
	};
}


