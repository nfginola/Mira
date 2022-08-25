#include "MeshManager.h"
#include "../RHI/RenderDevice.h"
#include "GPUGarbageBin.h"

namespace mira
{
    MeshManager::MeshManager(RenderDevice* device, GPUGarbageBin* bin, const SizeSpecification& size_spec) :
        m_rd(device),
        m_bin(bin)
    {
        assert(m_rd != nullptr);

        m_meshes.resize(1);

        // Assuming a number of maximum unique submeshes per manager for now
        constexpr u32 MAX_UNIQUE_SUBMESHES = 10'000;
        
        // Create device-local non-interleaved vertex buffers
        for (auto [attr, size] : size_spec.buffer_sizes)
        {
            auto& buffer = m_device_local_buffers[attr].buffer;
            auto& view = m_device_local_buffers[attr].full_view;
            auto& ator = m_device_local_buffers[attr].ator;
            const u32 count = size / get_stride(attr);

            buffer = m_rd->create_buffer(BufferDesc(size, MemoryType::Default));
            view = m_rd->create_view(buffer, BufferViewDesc(ViewType::ShaderResource, 0, get_stride(attr), count));
            ator = VirtualBlockAllocator(get_stride(attr), count);
        }

        // Create index buffer
        {
            auto& buffer = m_index_buffer.buffer;
            auto& view = m_index_buffer.full_view;
            auto& ator = m_index_buffer.ator;
            const u32 stride = sizeof(u32);
            const u32 count = size_spec.index_buffer_size / stride;

            buffer = m_rd->create_buffer(BufferDesc(size_spec.index_buffer_size, MemoryType::Default));
            view = m_rd->create_view(buffer, BufferViewDesc(ViewType::ShaderResource, 0, stride, count));
            ator = VirtualBlockAllocator(stride, count);
        }

        // Create staging buffer
        {
            m_staging_buffer.buffer = m_rd->create_buffer(BufferDesc(size_spec.staging_size, MemoryType::Upload));
            m_staging_buffer.ator = BumpAllocator(size_spec.staging_size, m_rd->map(m_staging_buffer.buffer));
        }

        // Create device-local submesh metadata buffer
        {
            const u32 size = MAX_UNIQUE_SUBMESHES * sizeof(SubmeshMetadata);

            m_submesh_metadata.buffer = m_rd->create_buffer(BufferDesc(size, MemoryType::Default));
            m_submesh_metadata.full_view = m_rd->create_view(m_submesh_metadata.buffer, BufferViewDesc(ViewType::ShaderResource, 0, sizeof(SubmeshMetadata), MAX_UNIQUE_SUBMESHES));
            m_submesh_metadata.ator = VirtualBlockAllocator(sizeof(SubmeshMetadata), MAX_UNIQUE_SUBMESHES);
        }
    }

    MeshContainer MeshManager::load_mesh(const MeshSpecification& spec)
    {
        Mesh_Storage storage{};
        RenderCommandList list;

        // Handle each attribute
        for (const auto& [attr, data] : spec.data)
        {
            auto total_size = data.size_bytes();

            // Upload to staging
            auto [mem, staging_offset] = m_staging_buffer.ator.allocate_with_offset(total_size);
            std::memcpy(mem, data.data(), total_size);

            // Reserve device-local memory
            auto dl_offset = m_device_local_buffers[attr].ator.allocate(total_size);

            // GPU-GPU copy
            list.submit(RenderCommandCopyBuffer(
                m_staging_buffer.buffer, staging_offset,
                m_device_local_buffers[attr].buffer, dl_offset,
                total_size));

            // Track device-local allocation
            storage.allocation_md[attr] = { dl_offset, total_size };
        }

        auto md_copy = spec.submeshes;
        for (const auto& submesh : spec.submeshes)
        {
            // Modify metadata for engine specific layout..
            // ...
            // ======================================================================== TO-DO
            // ...

            Submesh_Storage sm_storage{};
            sm_storage.md = submesh;

            storage.submeshes.push_back(sm_storage);
        }

        // Upload mesh metadata
        {
            const u64 total_size = sizeof(SubmeshMetadata) * spec.submeshes.size();

            // Grab staging memory and copy
            auto [mem, staging_offset] = m_staging_buffer.ator.allocate_with_offset(total_size);
            std::memcpy(mem, md_copy.data(), total_size);

            // Grab device-local memory
            const u64 dl_offset = m_submesh_metadata.ator.allocate(total_size);

            // Assign global index based on device-local position
            auto start = dl_offset;
            for (auto& sm_storage : storage.submeshes)
            {
                sm_storage.global_idx = (u32)(start / sizeof(SubmeshMetadata));
                start += sizeof(SubmeshMetadata);
            }

            // GPU-GPU copy
            list.submit(RenderCommandCopyBuffer(
                m_staging_buffer.buffer, staging_offset,
                m_submesh_metadata.buffer, dl_offset,
                total_size));

            // Track device-local allocation
            storage.submeshes_md_allocation = { dl_offset, total_size };
        }

        // Submit commands
        CommandList cmdls[]{ m_rd->allocate_command_list(QueueType::Copy) };
        m_rd->compile_command_list(cmdls[0], list);
        auto receipt = m_rd->submit_command_lists(cmdls, QueueType::Copy, {}, true);

        // CPU-wait
        m_rd->wait_for_gpu(*receipt);

        // Reset bump allocator
        m_staging_buffer.ator.clear();

        auto handle = m_handle_ator.allocate<Mesh>();
        try_insert(m_meshes, storage, get_slot(handle.handle));

        // Return helper container
        MeshContainer container{};
        container.mesh = handle;
        container.num_submeshes = (u32)spec.submeshes.size();
        container.manager_id = -1;                          // =============================== TO-DO

        return container;
    }

    void MeshManager::free_mesh(Mesh handle)
    {        
        auto& res = try_get(m_meshes, get_slot(handle.handle));

        auto deletion_func = [this, &res, handle]()
        {
            // Free device-local vertex data
            for (auto [attr, alloc_md] : res.allocation_md)
                m_device_local_buffers[attr].ator.free(alloc_md.first, alloc_md.second);

            // Free submeshes metadata
            m_submesh_metadata.ator.free(res.submeshes_md_allocation.first, res.submeshes_md_allocation.second);

            // Free internal mesh storage
            m_meshes[get_slot(handle.handle)] = std::nullopt;
            m_handle_ator.free(handle);
        };
          
        // Push to safe garbage bin
        m_bin->push_deferred_deletion(deletion_func);
    }

    Buffer MeshManager::get_index_buffer() const
    {
        return m_index_buffer.buffer;
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
    const SubmeshMetadata& MeshManager::get_submesh_metadata(Mesh mesh, u32 submesh) const
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


