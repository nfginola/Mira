#include "TextureManager.h"
#include "../RHI/RenderDevice.h"
#include "GPUGarbageBin.h"

namespace mira
{
	TextureManager::TextureManager(RenderDevice* rd, GPUGarbageBin* bin) :
		m_rd(rd),
		m_bin(bin)
	{
		m_textures.resize(1);

		/*
			The way the manager is setup currently (flush per texture load),
			the staging buffer is required to fit at least the largest texture load passed to this manager.
		*/

		constexpr u32 STAGING_SIZE = 200'000'000;
		m_staging = m_rd->create_buffer(BufferDesc(STAGING_SIZE, MemoryType::Upload));
		m_staging_ator = BumpAllocator(STAGING_SIZE, m_rd->map(m_staging));
	}
	TextureManager::~TextureManager()
	{
		for (auto& storage : m_textures)
		{
			if (storage.has_value())
			{
				m_rd->free_texture(storage->texture);
				m_rd->free_view(storage->view);
			}
		}
	}
	std::pair<LoadedTexture, u32> TextureManager::allocate(const std::string& name, const std::vector<TextureMipData>& image_data_mipped)
	{
		// If named exists: return existing
		auto it_existing = m_loaded_textures.find(name);
		if (it_existing != m_loaded_textures.cend())
		{
			auto hdl = it_existing->second;
			const auto& res = try_get(m_textures, get_slot(hdl.handle));
			return { hdl, m_rd->get_global_descriptor(res.view) };
		} 

		Texture_Storage storage{};
		auto handle = m_handle_ator.allocate<LoadedTexture>();

		// Create texture to fill
		TextureDesc td{};
		td.width = image_data_mipped[0].width;
		td.height = image_data_mipped[0].height;
		td.type = TextureType::Texture2D;
		td.mip_levels = (u32)image_data_mipped.size();
		td.memory_type = MemoryType::Default;
		td.format = ResourceFormat::RGBA_8_UNORM;				// @ we should fix to support SRGB
		storage.texture = m_rd->create_texture(td);

		// D3D12 requirement
		static constexpr auto TEX_ALIGNMENT = 512;

		// Record copy for each mip
		RenderCommandList list;
		for (u32 mip = 0; mip < image_data_mipped.size(); ++mip)
		{
			const TextureMipData& mip_data = image_data_mipped[mip];

			const u32 original_rowpitch = mip_data.width * sizeof(u32);									// Assuming 4 8-bit channels
			const u32 aligned_rowpitch = (1 + ((original_rowpitch - 1) / TEX_ALIGNMENT)) * TEX_ALIGNMENT;

			auto [staging_mem, staging_offset] = m_staging_ator.allocate_with_offset(aligned_rowpitch * mip_data.height);
			for (u32 row = 0; row < mip_data.height; ++row)
			{
				std::memcpy(
					staging_mem + row * aligned_rowpitch,
					mip_data.data.data() + row * original_rowpitch,
					original_rowpitch
				);
			}

			// Record GPU-GPU copy
			RenderCommandCopyBufferToImage cmd{};
			cmd.dst = storage.texture;
			cmd.src = m_staging;

			cmd.dst_subresource = mip;

			cmd.src_offset = staging_offset;
			cmd.src_rowpitch = aligned_rowpitch;
			cmd.src_width = mip_data.width;
			cmd.src_height = mip_data.height;
			cmd.src_format = td.format;
			cmd.src_depth = 1;

			list.submit(cmd);
		}

		// Execute
		CommandList cmdls[]{ m_rd->allocate_command_list() };
		m_rd->compile_command_list(cmdls[0], list);
		m_rd->submit_command_lists(cmdls);
		m_rd->flush();
		m_rd->recycle_command_list(cmdls[0]);
		m_staging_ator.clear();

		// Create view
		TextureViewDesc tvd(ViewType::ShaderResource, TextureViewRange(TextureViewDimension::Texture2D, ResourceFormat::RGBA_8_UNORM)
			.set_mips(0, (u32)image_data_mipped.size()));
		storage.view = m_rd->create_view(storage.texture, tvd);

		try_insert(m_textures, storage, get_slot(handle.handle));

		return { handle, m_rd->get_global_descriptor(storage.view) };
	}

	void TextureManager::free(LoadedTexture handle)
	{
		auto& res = try_get(m_textures, get_slot(handle.handle));
		m_bin->push_deferred_deletion([this, res_copy = res]()
			{
				m_rd->free_view(res_copy.view);
				m_rd->free_texture(res_copy.texture);
			});

		m_textures[get_slot(handle.handle)] = std::nullopt;
		m_handle_ator.free(handle);
	}
}

