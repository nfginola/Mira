#pragma once
#include "AssetResourceTypes.h"

namespace mira
{
	class TextureImporter
	{
	public:
		static void initialize();

		TextureImporter(const std::filesystem::path& path);

		std::shared_ptr<ImportedTexture> get_result();

	private:
		std::shared_ptr<ImportedTexture> m_result;
	};
}


