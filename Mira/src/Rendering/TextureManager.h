#pragma once
#include "../Common.h"
#include "../RHI/RenderResourceHandle.h"
#include "../RHI/RenderCommandList.h"
#include "../Handles/HandleAllocator.h"
#include "../Memory/BumpAllocator.h"
#include "../Resource/AssetResourceTypes.h"
#include "Types/TextureTypes.h"

namespace mira
{
	class RenderDevice;
	class GPUGarbageBin;

	class TextureManager
	{
	public:
		TextureManager(RenderDevice* rd, GPUGarbageBin* bin);

		// Currently only handles 2D mipped textures
		std::pair<LoadedTexture, u32> allocate(const std::string& name, const std::vector<TextureMipData>& image_data_mipped);
		void free(LoadedTexture handle);


	private:
		struct Texture_Storage
		{
			Texture texture;
			TextureView view;
		};

	private:
		RenderDevice* m_rd{ nullptr };
		GPUGarbageBin* m_bin{ nullptr };

		HandleAllocator m_handle_ator;

		std::vector<std::optional<Texture_Storage>> m_textures;
		std::unordered_map<std::string, LoadedTexture> m_loaded_textures;

		Buffer m_staging;
		BumpAllocator m_staging_ator;
	};
}
