#pragma once
#include "../Common.h"

namespace mira
{
	class VirtualRingBuffer
	{
	public:
		VirtualRingBuffer() = default;
		VirtualRingBuffer(u32 element_size, u32 element_count);
		
		u64 allocate();
		u64 pop();

		u32 get_element_size() const { return m_element_size; }

	private:
		bool is_full() const;
		bool is_empty() const;

	private:
		u64 m_total_size{ 0 };
		u32 m_element_size{ 0 };
		u32 m_element_count{ 0 };

		u64 m_head{ 0 };
		u64 m_tail{ 0 };
		bool m_full{ false };


	};
}


