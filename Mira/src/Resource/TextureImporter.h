#pragma once
#include "AssetResourceTypes.h"

namespace mira
{
	class TextureImporter
	{
	public:
		static void initialize();

		TextureImporter(const std::filesystem::path& path, bool generate_mips);

		std::shared_ptr<ImportedTexture> get_result();

	private:
		std::shared_ptr<ImportedTexture> m_result;
	};
}


