#pragma once
#include "../Common.h"
#include "AssetResourceTypes.h"

namespace mira
{
	struct ImportedModel;

	class AssimpImporter
	{
	public:
		AssimpImporter(const std::filesystem::path& path);

		std::shared_ptr<ImportedModel> get_result() const { return m_loaded_model; }


	private:
		std::shared_ptr<ImportedModel> m_loaded_model;

	};
}


