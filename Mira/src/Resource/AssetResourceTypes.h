#pragma once
#include "../Common.h"
#include "../Rendering/Types/MeshTypes.h"
#include "../Rendering/Types/MaterialTypes.h"

/*
	Inter-op structs
*/

namespace mira
{
	struct ImportedMaterial
	{
		std::unordered_map<MaterialTextureType, std::filesystem::path> textures;
	};

	struct ImportedMesh
	{
		std::unordered_map<VertexAttribute, std::vector<u8>> vertex_data;
		std::vector<u32> indices;
	};

	struct ImportedModel
	{
		std::vector<ImportedMaterial> materials;
		std::vector<SubmeshMetadata> submeshes;

		ImportedMesh mesh;
	};

	struct TextureMipData
	{
		std::vector<u8> data;
		u32 width{ 0 };
		u32 height{ 0 };
	};

	struct ImportedTexture
	{
		std::vector<TextureMipData> data_per_mip;
	};
}
