#include "MeshManager.h"

namespace mira
{
    MeshManager::MeshManager(RenderDevice* device, HandleAllocator* handle_ator, const SizeSpecification& size_spec) :
        m_rd(device),
        m_handle_ator(handle_ator)
    {
        constexpr u32 MAX_UNIQUE_SUBMESHES = 10'000;     // Assuming 5k unique submeshes per manager for now

        assert(m_rd != nullptr);
        assert(m_handle_ator != nullptr);

        m_meshes.resize(1);
        
        for (auto [attr, size] : size_spec.buffer_sizes)
        {
            // Create buffer
            auto& buffer = m_device_local_buffers[attr].buffer;
            buffer = handle_ator->allocate<Buffer>();

            BufferDesc desc{};
            desc.size = size;
            desc.memory_type = MemoryType::Default;
            m_rd->create_buffer(buffer, desc);

            // Create full view
            auto& view = m_device_local_buffers[attr].full_view;
            view = handle_ator->allocate<BufferView>();

            const u32 count = size / get_stride(attr);
            m_rd->create_view(view, buffer, 
                BufferViewDesc(ViewType::ShaderResource, 0, get_stride(attr), count));

            m_device_local_buffers[attr].ator = VirtualBlockAllocator(get_stride(attr), count);
        }

        // Create staging buffer
        {
            BufferDesc staging_d{};
            staging_d.size = size_spec.staging_size;
            staging_d.memory_type = MemoryType::Upload;

            m_staging_buffer.buffer = m_handle_ator->allocate<Buffer>();
            m_rd->create_buffer(m_staging_buffer.buffer, staging_d);

            m_staging_buffer.ator = BumpAllocator(
                staging_d.size,
                m_rd->map(m_staging_buffer.buffer)     // persistently map
            );        
        }

        // Create device-local submesh metadata buffer
        {
            BufferDesc submesh_md_d{};
            submesh_md_d.size = MAX_UNIQUE_SUBMESHES * sizeof(SubmeshMetadata);
            submesh_md_d.memory_type = MemoryType::Default;

            m_submesh_metadata.buffer = m_handle_ator->allocate<Buffer>();
            m_rd->create_buffer(m_submesh_metadata.buffer, submesh_md_d);

            m_submesh_metadata.full_view = handle_ator->allocate<BufferView>();
            m_rd->create_view(m_submesh_metadata.full_view, m_submesh_metadata.buffer,
                BufferViewDesc(ViewType::ShaderResource, 0, sizeof(SubmeshMetadata), MAX_UNIQUE_SUBMESHES));

            m_submesh_metadata.ator = VirtualBlockAllocator(sizeof(SubmeshMetadata), MAX_UNIQUE_SUBMESHES);
        }
    }
    std::pair<Mesh, u32> MeshManager::load_mesh(const MeshSpecification& spec)
    {
        // Handle each attribute
        for (const auto& [attr, data] : spec.data)
        {
            // Reserve memory in corresponding device-local buffer

            // Upload to staging

            // Push GPU-GPU copy callbacks
        }

        // Upload per submesh metadata to staging
        // Push GPU-GPU copy callback

        // Execute all registered GPU-GPU copies

        // CPU wait for completion


    }
    void MeshManager::free_mesh(Mesh handle)
    {
        /*
            Push the de-allocation from all virtual allocators for this mesh onto the GPUGarbageBin
        */
    }

    u32 MeshManager::get_attribute_buffer(VertexAttribute attr) const
    {
        return m_rd->get_global_descriptor(m_device_local_buffers.find(attr)->second.full_view);
    }

    u32 MeshManager::get_submesh_metadata_buffer() const
    {
        return m_rd->get_global_descriptor(m_submesh_metadata.full_view);
    }

    u32 MeshManager::get_submesh_metadata_index(Mesh mesh, u32 submesh) const
    {
        const auto& res = try_get(m_meshes, get_slot(mesh.handle));
        return res.submeshes[submesh].global_idx;
    }
    const MeshManager::SubmeshMetadata& MeshManager::get_submesh_metadata(Mesh mesh, u32 submesh) const
    {
        const auto& res = try_get(m_meshes, get_slot(mesh.handle));
        return res.submeshes[submesh].md;
    }
    u32 MeshManager::get_stride(VertexAttribute attr)
    {
        switch (attr)
        {
        case VertexAttribute::Position:
        case VertexAttribute::Normal:
        case VertexAttribute::Tangent:
            return 3 * sizeof(f32);
            break;
        case VertexAttribute::UV:
            return 2 * sizeof(f32);
            break;
        default:
            return 0;
            assert(false);
        }
    }
}


