#include "AssimpImporter.h"
#include <assimp/scene.h>           // Output data structure
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/postprocess.h>     // Post processing flags
#include <algorithm>

namespace mira
{
	std::filesystem::path extract_material(const aiMaterial* mat, MaterialTextureType type)
	{
		aiReturn ret{ aiReturn_SUCCESS };
		switch (type)
		{
		case MaterialTextureType::Diffuse:
		{
			aiString diffuse;
			ret = mat->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse);
			assert(ret == aiReturn_SUCCESS);

			return std::string(diffuse.C_Str());
		}
		case MaterialTextureType::Normal:
		{
			assert(false);
			break;
		}
		default:
			break;
		}

		return "";
	}

	AssimpImporter::AssimpImporter(const std::filesystem::path& path)
	{
		// Load assimp scene
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			path.relative_path().string().c_str(),
			aiProcess_Triangulate |

			// For Direct3D
			aiProcess_ConvertToLeftHanded |
			//aiProcess_FlipUVs |					// (0, 0) is top left
			//aiProcess_FlipWindingOrder |			// D3D front face is CW

			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |

			//aiProcess_PreTransformVertices

			// Extra flags (http://assimp.sourceforge.net/lib_html/postprocess_8h.html#a64795260b95f5a4b3f3dc1be4f52e410a444a6c9d8b63e6dc9e1e2e1edd3cbcd4)
			aiProcess_JoinIdenticalVertices |
			aiProcess_ImproveCacheLocality
		);

		if (!scene)
			assert(false);

		m_loaded_model = std::make_shared<ImportedModel>();
		m_loaded_model->mesh.vertex_data[VertexAttribute::Position] = {};
		m_loaded_model->mesh.vertex_data[VertexAttribute::UV] = {};
		m_loaded_model->mesh.vertex_data[VertexAttribute::Normal] = {};
		m_loaded_model->mesh.vertex_data[VertexAttribute::Tangent] = {};

		// Track material per submesh
		std::vector<u32> submesh_to_material_idx;
		std::vector<SubmeshMetadata>& submeshes = m_loaded_model->submeshes;

		// Load mesh
		{
			std::vector<u32>& indices = m_loaded_model->mesh.indices;
			std::vector<aiVector3D> positions, normals, tangents;
			std::vector<aiVector2D> uvs;

			// Reserve space
			{
				u32 total_verts{ 0 };
				for (u32 i = 0; i < scene->mNumMeshes; ++i)
					total_verts += scene->mMeshes[i]->mNumVertices;

				positions.reserve(total_verts);
				uvs.reserve(total_verts);
				normals.reserve(total_verts);
				tangents.reserve(total_verts);
			}

			// Go through all meshes
			for (uint32_t mesh_idx = 0; mesh_idx < scene->mNumMeshes; ++mesh_idx)
			{
				aiMesh* mesh = scene->mMeshes[mesh_idx];

				// Track submesh
				SubmeshMetadata submesh_md{};
				submesh_md.vert_start = (u32)positions.size();
				submesh_md.vert_count = mesh->mNumVertices;
				submesh_md.index_start = (u32)indices.size();
				submesh_md.index_count = 0;

				// Count indices
				for (u32 face_idx = 0; face_idx < mesh->mNumFaces; ++face_idx)
				{
					const aiFace& face = mesh->mFaces[face_idx];
					for (u32 index_idx = 0; index_idx < face.mNumIndices; ++index_idx)
						indices.push_back(face.mIndices[index_idx]);	
					submesh_md.index_count += face.mNumIndices;
				}
			
				// Grab per vertex data
				for (u32 vert_idx = 0; vert_idx < mesh->mNumVertices; ++vert_idx)
				{
					positions.push_back({ mesh->mVertices[vert_idx].x, mesh->mVertices[vert_idx].y, mesh->mVertices[vert_idx].z });

					if (mesh->HasTextureCoords(0))
						uvs.push_back({ mesh->mTextureCoords[0][vert_idx].x, mesh->mTextureCoords[0][vert_idx].y });

					if (mesh->HasNormals())
						normals.push_back({ mesh->mNormals[vert_idx].x, mesh->mNormals[vert_idx].y, mesh->mNormals[vert_idx].z });

					if (mesh->HasTangentsAndBitangents())
					{
						tangents.push_back({ mesh->mTangents[vert_idx].x, mesh->mTangents[vert_idx].y, mesh->mTangents[vert_idx].z });
						//bitangents.push_back({ mesh->mBitangents[vert_idx].x, mesh->mBitangents[vert_idx].y, mesh->mBitangents[vert_idx].z });
					}
				}

				// Track material
				submesh_to_material_idx.push_back(mesh->mMaterialIndex);

				// Track submesh
				submeshes.push_back(submesh_md);
			}

			// Resize standardized buffers
			m_loaded_model->mesh.vertex_data[VertexAttribute::Position].resize(positions.size() * sizeof(positions[0]));
			m_loaded_model->mesh.vertex_data[VertexAttribute::UV].resize(uvs.size() * sizeof(uvs[0]));
			m_loaded_model->mesh.vertex_data[VertexAttribute::Normal].resize(normals.size() * sizeof(normals[0]));
			m_loaded_model->mesh.vertex_data[VertexAttribute::Tangent].resize(tangents.size() * sizeof(tangents[0]));

			// Copy to standardized buffers
			std::memcpy(m_loaded_model->mesh.vertex_data[VertexAttribute::Position].data(), positions.data(), positions.size() * sizeof(positions[0]));
			std::memcpy(m_loaded_model->mesh.vertex_data[VertexAttribute::UV].data(), uvs.data(), uvs.size() * sizeof(uvs[0]));
			std::memcpy(m_loaded_model->mesh.vertex_data[VertexAttribute::Normal].data(), normals.data(), normals.size() * sizeof(normals[0]));
			std::memcpy(m_loaded_model->mesh.vertex_data[VertexAttribute::Tangent].data(), tangents.data(), tangents.size() * sizeof(tangents[0]));
		}

		// Sanity check
		assert(submeshes.size() == submesh_to_material_idx.size());

		// Extract material load data
		const auto directory = path.parent_path().string();
		for (auto mat_id : submesh_to_material_idx)
		{
			ImportedMaterial mat_md{};
			const aiMaterial* material = scene->mMaterials[mat_id];
			
			mat_md.textures[MaterialTextureType::Diffuse] = directory + extract_material(material, MaterialTextureType::Diffuse).string();

			// Save material
			m_loaded_model->materials.push_back(mat_md);
		}
	}
}
