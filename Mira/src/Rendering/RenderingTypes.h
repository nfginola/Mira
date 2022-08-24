#pragma once
#include "../Common.h"

namespace mira
{
	class TypedHandlePool;

	struct Mesh { friend TypedHandlePool; u64 handle{ 0 }; };			
	struct Material { friend TypedHandlePool; u64 handle{ 0 }; };

}