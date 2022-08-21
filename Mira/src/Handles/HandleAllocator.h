#pragma once
#include "TypedHandlePool.h"
#include <unordered_map>
#include <typeindex>

namespace mira
{
	class HandleAllocator
	{
	public:	
		template <typename Handle>
		Handle allocate()
		{
			// creates new pool if none exists, otherwise uses existing
			auto& pool = m_pools[typeid(Handle)];
			return pool.allocate_handle<Handle>();
		}

		template <typename Handle>
		void free(Handle&& handle)
		{
			auto& pool = m_pools[typeid(Handle)];
			pool.free_handle<Handle>(handle);
		}

	private:
		std::unordered_map<std::type_index, TypedHandlePool> m_pools;

	};


}
