#pragma once
#include "../../Common.h"

namespace mira
{
	struct Mesh { friend class TypedHandlePool; u64 handle{ 0 }; };

	enum class VertexAttribute
	{
		Position,
		Normal,
		UV,
		Tangent
	};

	struct SubmeshMetadata
	{
		u32 vert_start{ 0 };
		u32 vert_count{ 0 };
		u32 index_start{ 0 };
		u32 index_count{ 0 };
	};

	struct MeshContainer
	{
		Mesh mesh;
		u32 num_submeshes{ 0 };

		u32 manager_id{ UINT_MAX };
	};
}
