#include "TextureImporter.h"
#include "compressonator.h"

/*
	Requires installing CompressonatorFramework
	https://github.com/GPUOpen-Tools/Compressonator/releases/tag/V4.2.5185

	They will give you prebuilt libs which you can copy over! :)
*/

namespace mira
{
	void TextureImporter::initialize()
	{
		CMP_InitFramework();
	}

	TextureImporter::TextureImporter(const std::filesystem::path& path, bool generate_mips)
	{
		CMP_MipSet mip_set_in;
		memset(&mip_set_in, 0, sizeof(CMP_MipSet));
		auto cmp_status = CMP_LoadTexture(path.string().c_str(), &mip_set_in);
		if (cmp_status != CMP_OK) 
		{
			std::printf("Error %d: Loading source file!\n", cmp_status);
			assert(false);
		}

		CMP_INT mip_requests = 1;

		if (generate_mips)
		{
			constexpr u32 MAX_MIPS_REQUEST = 3;		// arbitrary number, for now.

			mip_requests = MAX_MIPS_REQUEST;
			mip_requests = (std::min)(mip_requests, mip_set_in.m_nMaxMipLevels);
			if (mip_set_in.m_nMipLevels <= 1)
			{
				//------------------------------------------------------------------------
				// Checks what the minimum image size will be for the requested mip levels
				// if the request is too large, a adjusted minimum size will be returns
				//------------------------------------------------------------------------
				CMP_INT n_min_size = CMP_CalcMinMipSize(mip_set_in.m_nHeight, mip_set_in.m_nWidth, mip_requests);

				//--------------------------------------------------------------
				// now that the minimum size is known, generate the miplevels
				// users can set any requested minumum size to use. The correct
				// miplevels will be set acordingly.
				//--------------------------------------------------------------
				CMP_GenerateMIPLevels(&mip_set_in, n_min_size);
			}
		}
		

		m_result = std::make_shared<ImportedTexture>();
	
		// Grab mips
		for (u32 i = 0; i < mip_requests; ++i)
		{
			CMP_MipLevel* mip{ nullptr };
			CMP_GetMipLevel(&mip, &mip_set_in, i, 0);

			std::vector<u8> data;
			data.resize(mip->m_dwLinearSize);
			std::memcpy(data.data(), mip->m_pbData, mip->m_dwLinearSize);

			TextureMipData mip_data{};
			mip_data.data = std::move(data);
			mip_data.width = mip->m_nWidth;
			mip_data.height = mip->m_nHeight;

			m_result->data_per_mip.push_back(std::move(mip_data));
		}
	}

	std::shared_ptr<ImportedTexture> TextureImporter::get_result()
	{
		return m_result;
	}
}
