#pragma once
#include "../Common.h"

class TypedHandlePool;

namespace mira
{
	struct RenderResourceHandle
	{
	public:
		u32 key() const { return (u32)(m_handle & SLOT_MASK); };

	private:
		friend TypedHandlePool;

		static const u64 SLOT_MASK = ((u64)1 << std::numeric_limits<uint32_t>::digits) - 1; // Mask of the lower 32-bits
		u64 m_handle;
	};

	struct Buffer : public RenderResourceHandle { friend TypedHandlePool; };
	struct Texture : public RenderResourceHandle { friend TypedHandlePool; };
	struct Pipeline : public RenderResourceHandle { friend TypedHandlePool; };
	struct RenderPass : public RenderResourceHandle { friend TypedHandlePool; };
}

