//#pragma once
//#include "Types/MeshTypes.h"
//#include "../RHI/RHITypes.h"
//#include "../Handles/HandleAllocator.h"
//#include "../Memory/BumpAllocator.h"			// For staging buffer sub-allocation
//#include "../Memory/VirtualBlockAllocator.h"	// For device-local buffer sub-allocation
//
//namespace mira
//{
//	class RenderDevice;
//
//	class MeshManager
//	{
//	public:
//		enum class VertexAttribute
//		{
//			Position,
//			Normal,
//			UV,
//			Tangent
//		};
//
//		// Identifies a submesh
//		struct SubmeshMetadata
//		{
//			u32 vert_start{ 0 };
//			u32 vert_count{ 0 };
//			u32 index_start{ 0 };
//			u32 index_count{ 0 };
//		};
//
//		// Assumes the mesh data to be a concatenation of multiple submeshes
//		struct MeshSpecification
//		{
//			std::unordered_map<VertexAttribute, std::span<u8>> data;
//			std::span<SubmeshMetadata> submeshes;
//		};
//
//		struct SizeSpecification
//		{
//			std::unordered_map<VertexAttribute, u32> buffer_sizes;
//			u32 staging_size{ 0 };
//		};
//
//	public:
//		MeshManager(mira::RenderDevice* device, HandleAllocator* handle_ator, const SizeSpecification& size_spec);
//
//		// Returns mesh handle and number of submeshes
//		std::pair<Mesh, u32> load_mesh(const MeshSpecification& spec);
//
//		void free_mesh(Mesh handle);
//
//		// Get GPU-indexable identifier for per attribute buffer
//		u32 get_attribute_buffer(VertexAttribute attr) const;
//
//		// Get GPU-indexable identifier for mesh metadata buffer
//		u32 get_submesh_metadata_buffer() const;
//
//		// Get global index within mesh metadata buffer for this specific submesh
//		u32 get_submesh_metadata_index(Mesh mesh, u32 submesh) const;
//
//		// Grab metadata on CPU-side
//		const SubmeshMetadata& get_submesh_metadata(Mesh mesh, u32 submesh) const;
//
//	private:
//		struct Submesh_Storage
//		{
//			SubmeshMetadata md;
//
//			// Calculated through each allocation on the device-local submesh metadata
//			u32 global_idx{ 0 };		
//		};
//
//		struct Mesh_Storage
//		{
//			std::vector<Submesh_Storage> submeshes;
//		};
//
//		// Non-interleaved vertex data.
//		struct DeviceLocal_Buffer
//		{
//			mira::Buffer buffer;
//			mira::BufferView full_view;
//			VirtualBlockAllocator ator;
//		};
//
//		struct Staging_Buffer
//		{
//			mira::Buffer buffer;
//			BumpAllocator ator;
//		};
//
//	private:
//		u32 get_stride(VertexAttribute attr);
//
//	private:
//		mira::RenderDevice* m_rd{ nullptr };
//		HandleAllocator* m_handle_ator;
//
//		std::vector<std::optional<Mesh_Storage>> m_meshes;
//
//		std::unordered_map<VertexAttribute, DeviceLocal_Buffer> m_device_local_buffers;
//		DeviceLocal_Buffer m_submesh_metadata;
//		Staging_Buffer m_staging_buffer;
//
//		// Upload technique is to submit copy to copy queue and immediately flush after each mesh load for simplicity
//		mira::CommandList m_cmdl;
//	};
//}
//
//
