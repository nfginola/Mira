#pragma once
#include "../Common.h"
#include "VirtualRingBuffer.h"

namespace mira
{
	class RingBuffer
	{
	public:
		RingBuffer() = default;
		RingBuffer(u32 element_size, u32 element_count, u8* memory = nullptr);
		~RingBuffer();

		u8* allocate();
		std::pair<u8*, u64> allocate_with_offset();
		u8* pop();

		u32 get_element_size() const { return m_element_size; }

	private:
		u8* m_heap_start{ nullptr };

		u32 m_element_size{ 0 };
		bool m_is_internally_managed{ false };

		VirtualRingBuffer m_vator;
	};
}


