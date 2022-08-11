#pragma once
#include "../Common.h"

class TypedHandlePool;

namespace mira
{
	struct Buffer { friend TypedHandlePool; u64 handle{ 0 }; };
	struct Texture  { friend TypedHandlePool; u64 handle{ 0 }; };
	struct Pipeline { friend TypedHandlePool; u64 handle{ 0 }; };
	struct RenderPass { friend TypedHandlePool; u64 handle{ 0 }; };

	static u32 get_slot(u64 handle)
	{
		static const u64 SLOT_MASK = ((u64)1 << std::numeric_limits<uint32_t>::digits) - 1; // Mask of the lower 32-bits
		return (u32)(handle & SLOT_MASK);
	}
}

