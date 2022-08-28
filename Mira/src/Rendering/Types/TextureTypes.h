#pragma once
#include "../../Common.h"

namespace mira
{
	struct LoadedTexture { friend class TypedHandlePool; u64 handle{ 0 }; };
}
