#pragma once
#include "../../Common.h"

namespace mira
{
	struct PersistentConstant { friend class TypedHandlePool; u64 handle{ 0 }; };

}
