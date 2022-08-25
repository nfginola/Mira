#pragma once
#include "../../Common.h"

namespace mira
{
	struct Mesh { friend class TypedHandlePool; u64 handle{ 0 }; };



	struct MeshContainer
	{
		Mesh mesh;
		u32 num_submeshes{ 0 };

		u32 manager_id{ -1 };
	};
}
