#pragma once
#include "../Common.h"

namespace mira
{
	class VirtualBumpAllocator
	{
	public:
		VirtualBumpAllocator() = default;
		VirtualBumpAllocator(u64 size) : m_size(size) {}

		[[nodiscard]] u64 allocate(u64 size, u16 alignment = 0)
		{
			// Bump to aligned address
			const u64 to_align = alignment == 0 ? 0 : alignment - (m_head % alignment);
			m_head += to_align;

			const u64 start = m_head;
			m_head += size;

			assert(m_head < m_size);

			return start;
		}

		void clear()
		{
			m_head = 0;
		}

	private:
		u64 m_size{ 0 };

		u64 m_head{ 0 };
	};
}


