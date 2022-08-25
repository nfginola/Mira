#include "AssimpImporter.h"
#include <assimp/scene.h>           // Output data structure
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/postprocess.h>     // Post processing flags
#include <algorithm>

namespace mira
{
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

			// Copy to standardized buffers
			std::copy(positions.begin(), positions.end(), std::back_inserter(m_loaded_model->mesh.vertex_data[VertexAttribute::Position]));
			std::copy(uvs.begin(), uvs.end(), std::back_inserter(m_loaded_model->mesh.vertex_data[VertexAttribute::UV]));
			std::copy(normals.begin(), normals.end(), std::back_inserter(m_loaded_model->mesh.vertex_data[VertexAttribute::Normal]));
			std::copy(tangents.begin(), tangents.end(), std::back_inserter(m_loaded_model->mesh.vertex_data[VertexAttribute::Tangent]));
		}

		// Handle material
		{
			

		}



		//u32 num_meshes = scene->mNumMeshes;
		//u32 num_materials = scene->mNumMaterials;
		//u32 num_textures = scene->mNumTextures;

		//// Get total amount of vertices
		//u32 total_verts = 0;
		//for (u32 i = 0; i < scene->mNumMeshes; ++i)
		//	total_verts += scene->mMeshes[i]->mNumVertices;

		//// Extract geometry data
		//for (uint32_t mesh_idx = 0; mesh_idx < scene->mNumMeshes; ++mesh_idx)
		//{
		//	aiMesh* mesh = scene->mMeshes[mesh_idx];

		//	// Assimp 'mesh' is translated to a submesh for this implementation
		//	SubmeshMetadata submesh{};
		//	uint32_t& vertex_start = submesh.vert_start;
		//	uint32_t& vertex_count = submesh.vert_count;
		//	uint32_t& index_start = submesh.index_start;
		//	uint32_t& index_count = submesh.index_count;

		//	ImportedMaterial material{};

		//	vertex_start = (uint32_t)m_mesh.positions.size();
		//	vertex_count = mesh->mNumVertices;
		//	index_start = (uint32_t)m_mesh.indices.size();
		//	index_count = 0;

		//	// Pair each mesh with material
		//	//submesh.mat_idx = mesh->mMaterialIndex;
		//	submesh.mat_idx = original_to_new_idx[mesh->mMaterialIndex];

		//	for (uint32_t face_idx = 0; face_idx < mesh->mNumFaces; ++face_idx)
		//	{
		//		const aiFace& face = mesh->mFaces[face_idx];
		//		for (uint32_t index_idx = 0; index_idx < face.mNumIndices; ++index_idx)
		//		{
		//			m_mesh.indices.push_back(face.mIndices[index_idx]);

		//			// add vertex start here to avoid adding vertex_start manually during rendering!
		//			//m_mesh.indices.push_back(face.mIndices[index_idx] + vertex_start);		

		//		}
		//		index_count += face.mNumIndices;
		//	}

		//	for (uint32_t vert_idx = 0; vert_idx < mesh->mNumVertices; ++vert_idx)
		//	{
		//		m_mesh.positions.push_back({ mesh->mVertices[vert_idx].x, mesh->mVertices[vert_idx].y, mesh->mVertices[vert_idx].z });

		//		if (mesh->HasTextureCoords(0))
		//			m_mesh.uvs.push_back({ mesh->mTextureCoords[0][vert_idx].x, mesh->mTextureCoords[0][vert_idx].y });

		//		if (mesh->HasNormals())
		//			m_mesh.normals.push_back({ mesh->mNormals[vert_idx].x, mesh->mNormals[vert_idx].y, mesh->mNormals[vert_idx].z });

		//		if (mesh->HasTangentsAndBitangents())
		//		{
		//			m_mesh.tangents.push_back({ mesh->mTangents[vert_idx].x, mesh->mTangents[vert_idx].y, mesh->mTangents[vert_idx].z });
		//			m_mesh.bitangents.push_back({ mesh->mBitangents[vert_idx].x, mesh->mBitangents[vert_idx].y, mesh->mBitangents[vert_idx].z });
		//		}
		//	}

		//	m_mesh.submeshes.push_back(submesh);
		//}
	}
}
