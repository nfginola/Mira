#include "VirtualRingBuffer.h"

namespace mira
{

	VirtualRingBuffer::VirtualRingBuffer(u32 element_size, u32 element_count) : 
		m_total_size((u64)element_size * element_count),
		m_element_size(element_size),
		m_element_count(element_count)
	{
	}

	u64 VirtualRingBuffer::allocate()
	{
		const u64 requested_start = m_head;
		const u64 offset = requested_start * m_element_size;
		const u64 next_head = (requested_start + 1) % m_element_count;

		if (requested_start == m_tail && is_full())
			return (u64)-1;

		m_head = next_head;
		m_full = next_head == m_tail;

		return offset;
	}

	u64 VirtualRingBuffer::pop()
	{
		if (is_empty())
			return (u64)-1;

		const u64 offset = m_tail * m_element_size;

		m_tail = (m_tail + 1) % m_element_count;
		m_full = false;

		return offset;
	}

	bool VirtualRingBuffer::is_full() const
	{
		return m_full;
	}

	bool VirtualRingBuffer::is_empty() const
	{
		return !m_full && (m_head == m_tail);
	}
}

