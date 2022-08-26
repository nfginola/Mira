#include "RingBuffer.h"

namespace mira
{
	RingBuffer::RingBuffer(u32 element_size, u32 element_count, u8* memory) :
		m_heap_start(memory),
		m_element_size(element_size),
		m_is_internally_managed(memory == nullptr ? true : false)
	{
		if (!memory)
		{
			m_heap_start = (u8*)std::malloc(element_size * element_count);
			if (m_heap_start)
				std::memset(m_heap_start, 0, element_size * element_count);
		}

		m_vator = VirtualRingBuffer(element_size, element_count);
	}

	RingBuffer::~RingBuffer()
	{
		if (m_is_internally_managed)
			std::free(m_heap_start);
	}

	u8* RingBuffer::allocate()
	{
		auto offset = m_vator.allocate();
		if (offset == (u64)-1)
			return nullptr;
		return m_heap_start + offset;
	}

	std::pair<u8*, u64> RingBuffer::allocate_with_offset()
	{
		auto offset = m_vator.allocate();
		if (offset == (u64)-1)
			return { nullptr, (u64)-1 };
		return { m_heap_start + offset, offset };
	}
	
	u8* RingBuffer::pop()
	{
		auto offset = m_vator.pop();
		if (offset == (u64)-1)
			return nullptr;
		return m_heap_start + offset;
	}
}
